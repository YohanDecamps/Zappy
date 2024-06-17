#!/bin/python
"""
Module providing the trantorian class
"""
from enum import IntEnum
from time import time
import random
from warnings import warn

from multiprocessing import Queue
from client.client import Client

from parser.concrete.message_type_parser import MessageType
from parser.concrete.message_type_parser import MessageTypeParser
from trentorian.map import (
    create_default_map,
    Map,
    MapTile,
    merge_maps
)

from utils import (
    determine_direction,
    check_levelup,
    pack_infos,
    unpack_infos,
    split_list,
    LEVELS,
    get_incantation_team
)

# TODO use init file for the module

#-----------------------------------------------------------------------------#
############################### THE CLASS #####################################
#-----------------------------------------------------------------------------#

FOOD = 126
class TrantorianDirection(IntEnum):
    """Trantorian direction enum
    """
    UNKWOW = -1
    RIGHT = 0
    UP = 1
    LEFT = 2
    DOWN = 3

    def get_offset(self) -> tuple[int, int]:
        """get the offset to add to a position

        Returns:
            tuple[int, int]: x and y to change
        """
        if self == 0: # RIGHT
            return 1, 0
        if self == 1: # UP
            return 0, -1
        if self == 2: # LEFT
            return -1, 0
        if self == 3: # DOWN
            return 0, 1
        return 0, 0

    def __str__(self) -> str:
        if self == 0:
            return "RIGHT"
        if self == 1:
            return "UP"
        if self == 2:
            return "LEFT"
        if self == 3:
            return "DOWN"
        return "UNKWOW"

class SoundDirection(IntEnum):
    """Sound direction enum
    """
    EAST = 0
    NORTH_EAST = 1
    NORTH = 2
    NORTH_WEST = 3
    WEST = 4
    SOUTH_WEST = 5
    SOUTH = 6
    SOUTH_EAST = 7

class Trantorian:
    """Class for a trantorian
    """
    def __init__(self, host: str, port: int, team: str):
        self.host = host
        self.port = port
        try:
            self.client = Client(host, port, team)
        except (RuntimeError, ConnectionRefusedError) as err:
            raise RuntimeError("Trentor didn't connect to server") from err
        self.team: str = team
        self.level: int = 1
        self.x: int = 0
        self.y: int = 0
        self.dead: bool = False
        self.unused_slot: int = 1
        self.others: dict = {} # uid : (inv, lvl, x, y, last update, free for incant)
        self.received_messages: list = []
        self.uid: str = str(time())
        self.state: str = ""
        self.dispo_incant: bool = True # disponibility for an incantation
        self.inventory: dict = {
            "food": 10, "linemate": 0, "deraumere": 0,
            "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0
        }
        self.known_map = create_default_map(self.client.size_x, self.client.size_y)
        self.direction: TrantorianDirection = TrantorianDirection.UP
        self.has_already_reiceived_birth_info: bool = False
        self.last_birthed: bool = True
        self.egg_pos: tuple = (0, 0)
        self.team_size: int = 0
        self.last_msg_infos: list = []
        self.last_beacon_direction: int = 0
        self.last_beacon_uid: str = None
        self.last_beacon_message_uid: str = None
        self.number_of_ritual_ready: int = 0
        # least amount of food needed to go throught the longest distance and do an incantation
        self.mini_food: int = 20 + ((self.known_map.width + self.known_map.height) * 8 / FOOD)

    def born(self, queue: Queue):
        """launch a trantorian and do its life

        Args:
            queue (Queue): process shared queu (for birth)
        """
        try:
            print("live")
            self.start_living(queue)
            self.first_level()
            while self.iter_food() and self.level: # TODO deadlook if two calls the same
                can_level_up = self.wander()
                if self.state == "shaman":
                    print(self.uid, "start incant with:", can_level_up)
                    a = self.be_the_shaman(can_level_up)
                    self.broadcast(MessageTypeParser().serialize(MessageType.RITUAL_FINISH, self), ["all"])
                    print(self.uid, "incantation end", a)
                if self.state == 'going somewhere':
                    self.follow_the_leader()
                self.broadcast("just$update", ["all"])
        except BrokenPipeError:
            print("Server closed socket")
            self.dead = True
        print("died")
        return

    def be_the_shaman(self, can_do_incant: list) -> bool:
        """call the others and do the incantation process

        Args:
            can_do_incant (list): trants to do the incantation with

        Returns:
            bool: incantation's success
        """
        self.beacon(can_do_incant)
        if self.state != "ready": # in case an other one calls him
            return False

        self.drop_stuff()
        for _ in range(4): #TODO now wait for all to drop their stuff
            self.look_around()
        # TODO look to check the conditions
        return self.start_incantation()


    def follow_the_leader(self) -> bool:
        """go and do the incantation whit the one who called

        Returns:
            bool: incantation's succes
        """
        print(self.uid, 'Follow leader', self.last_beacon_uid)
        self.follow_beacon()
        self.drop_stuff() # TODO send message to tell we are ready
        ans = ""
        print(self.uid, "wait incant", self.last_beacon_uid)
        while not self.dead and not ans.startswith("Current level"): # TODO do that better
            ans = self.wait_answer()
            if ans == "incantation end":
                print(self.uid, "Incantation failed")
                return False
        if self.dead or len(ans) != 16:
            return False
        self.broadcast(MessageTypeParser().serialize(MessageType.RITUAL_FINISH, self), "all")
        print(self.uid, "incant end", ans)
        lvl = int(ans[15]) # TODO do this shit better
        print("lvl:", lvl)
        self.level = int(ans[15])
        print(self.uid, "incantation done new level: ", lvl)
        return True

    def first_level(self) -> None:
        """script to fastly go to level 2 and acquire some food
        """
        ready: bool = False
        while self.iter_food() and not ready: # TODO look to know where there is linemate
            self.forward()
            self.take_object("linemate")
            direct: int = random.randint(0, 5)
            if direct == 1:
                self.left()
            elif direct == 2:
                self.right()
            ready = check_levelup(self.inventory, 2, {})
        if self.dead:
            return
        if self.inventory["linemate"] == 0:
            self.first_level()
            return
        while not self.dead and not self.drop_object("linemate"):
            continue
        if self.start_incantation() is False:
            self.first_level()
        return

##############################  UTILS  #######################################

    def iter(self) -> bool:
        """update the needed values and check if the next iteration is possible (not dead)

        Returns:
            bool: can do the next iteration
        """
        if self.dead:
            return False
        self.get_unused_slot()
        return True

    def iter_food(self) -> bool:
        """check if next iteration is possible, and update the needed values
        if the food is to low, refills it to 15 unit

        Returns:
            bool: can do the net iteration
        """
        if not self.iter():
            return False
        self.get_inventory()
        if self.inventory["food"] > self.mini_food:
            return True
        max_reached: bool = True
        state_change: bool = True
        under_mini: bool = True
        while self.iter() and (under_mini or (max_reached and state_change)):
            if self.dead:
                return False
            if not self.look_around():
                continue
            for _ in range(self.level): # this can be optimised
                for _ in range(self.get_current_case().content["food"]):
                    if not self.take_object("food"):
                        break
                if self.dead:
                    return False
                self.forward()
            direct: int = random.randint(0, 5)
            if direct == 1:
                self.left()
            elif direct == 2:
                self.right()
            self.get_inventory()
            max_reached: bool =  self.inventory["food"] < self.mini_food + 10
            state_change: bool = self.state != "going somewhere"
            under_mini: bool = self.inventory["food"] < self.mini_food
        return not self.dead

    def get_current_case(self) -> MapTile:
        """get the case we are one

        Returns:
            MapTile: current maptile
        """
        return self.known_map.tiles[self.y % self.client.size_y][self.x % self.client.size_x]

    def wander(self) -> list[str]:
        """look and take stuff until we can do an incantation, or we are called

        Returns:
            list[str]: list of players to do the incantation
        """
        self.state = "wander"
        can_level_up = []
        i = 0
        while self.iter_food() and self.state == "wander" and not can_level_up:
            self.look_around()
            for _ in range(self.level): # TODO maybe some pririty order here
                if not self.take_tile_objects():
                    break
                self.forward()
            direct: int = random.randint(0, 5)
            if direct == 1:
                self.left()
            elif direct == 2:
                self.right()
            can_level_up = get_incantation_team(self.inventory, self.level + 1, self.others)
            i += 1
            if i % 4:
                self.broadcast("just$update", ["all"])
        if can_level_up and self.state != "going somewhere":
            self.state = "shaman"
        return can_level_up

    def follow_beacon(self) -> None:
        """follow the beacon to make a ritual
        """
        while self.iter() and self.state == "going somewhere":
            if self.last_beacon_direction == 0:
                self.state = "ritual_prep"
                break

            if self.last_beacon_direction < 5 and self.last_beacon_direction != 1:
                self.left()
            elif self.last_beacon_direction == 5:
                self.left()
                self.left()
            elif self.last_beacon_direction > 5:
                self.right()

            self.forward()
            # self.look_around()

        self.broadcast(MessageTypeParser().serialize(MessageType.RITUAL_READY, self), [self.last_beacon_uid])

    def beacon(self, receivers: list) -> None:
        """set ourselves as a beacon to atract the others

        Args:
            receivers (list): people to regroup
        """
        self.state = "beacon"

        while self.iter() and self.state == "beacon":
            self.broadcast(MessageTypeParser().serialize(MessageType.BEACON, self), receivers)
            if self.number_of_ritual_ready >= LEVELS[self.level + 1][0] - 1:
                print("we are ready")
                self.state = "ready"

    def start_living(self, queue: Queue) -> None:
        """ First step of the life (reproduction)

        Args:
            queue (Queue): queue to birth other trantorians
        """
        self.iter_food()
        self.broadcast("im$alive", ["all"])
        self.get_unused_slot()
        b2, b3 = True, True
        team_size = len(self.others) + 1
        nbr = 0
        while self.iter_food() and b2 and b3:
            # the objective of all those checks is to work even if the server is broken
            b2 = team_size < self.client.team_size and nbr < 2
            b3 = team_size < 5 and self.unused_slot > 0 # TODO must be 8
            new_size = len(self.others.items()) + 1
            if new_size == team_size:
                nbr += 1
            team_size = new_size
            self.get_unused_slot()
            self.asexual_multiplication(queue)
            self.broadcast("im$alive", ["all"])
        self.broadcast("ready$tolive", ["all"])
        return

    def suicide(self) -> None:
        """look around until we are dead
        """
        while not self.dead:
            self.look_around()
        return

    def take_tile_objects(self) -> bool:
        """take all the objects on the tile

        Returns:
            bool: true if all the object where taken, false if one didn't work
        """
        succes: bool = True
        content: dict = self.get_current_case().content
        for obj, quantity in content.items():
            if obj == "egg" or obj == "player":
                continue
            for _ in range(quantity):
                if self.take_object(obj):
                    succes = False
        return succes

    def merge_others(self, received: dict) -> None:
        """merge a received list of user into the current one

        Args:
            received (dict): received informations
        """
        for uuid, infos in received.items():
            update = infos[4]
            if uuid not in self.others:
                self.others[uuid] = infos
                continue
            if update > self.others[uuid][3]:
                self.others[uuid] = infos
        return

    def wait_answer(self) -> str: # TODO, receive "Current level" from an incantation ?"
        """wait for an answer, handle the message and eject

        Returns:
            str: last server answer
        """
        answer: str = self.client.get_answer()
        # TODO replace with startwith
        while answer[:7] == 'message' or answer[:5] == 'eject':
            if answer[:7] == 'message':
                self.receive_message(answer)
            if answer[:5] == 'eject':
                self.handle_eject(answer)
            answer = self.client.get_answer()
        if answer[:4] == "dead":
            self.dead = True
        return answer


    def receive_message(self, msg: str) -> None: # TODO remove the map
        """handle the broadcast reception
            our messages are composed of :
            "sender_uuid$receiver_uuid$type$content$infos$map"

        Args:
            msg (str): message sent by the server
        """
        msg = msg[8:]
        direct, msg = msg.split(',')
        direct: int = int(direct)
        parts = msg.strip().split("$")
        if len(parts) != 5:
            self.received_messages.append(msg)
            return
        sender, receivers, msg_type, content, infos = parts
        if msg_type == MessageType.BEACON:
            for uid in receivers.split('|'):
                self.others[uid][5] = False
        if receivers != "all" and self.uid not in receivers.split('|'):
            return
        info_unpacked = unpack_infos(infos, self.uid)
        self.merge_others(info_unpacked)
        try:
            MessageTypeParser().deserialize(self, int(msg_type), content, direct)
            self.last_msg_infos = [sender, msg_type, content]
        except (KeyError, ValueError):
            return
        MessageTypeParser().deserialize(self, int(msg_type), content, direct)
        self.last_msg_infos = [sender, msg_type, content]
        return


    def handle_eject(self, msg: str) -> None:
        """handle an ejection

        Args:
            msg (str): message sent by the server
        """
        if len(msg) != 8:
            return
        direct: TrantorianDirection = TrantorianDirection(int(msg[-1]))
        off_x, off_y = direct.get_offset()
        self.x = (self.x - off_x) % self.client.size_x # TODO check if the direction is correct
        self.y = (self.y - off_y) % self.client.size_y
        return


    def update_direction_from_msg(self, sender_x: int, sender_y: int, sound_direction: int) -> bool:
        # TODO get previous line parameter from message parsing when a protocol will be implemented
        """get the direction from the sound direction

        Args:
            sender_x (int): x of the sender
            sender_y (int): y of the sender
            sound_direction (int): direction of the sound

        Returns:
            bool: true if it could be located, otherwise false
        """

        if sender_x == 0 and sender_y == 0:
            return False

        predicted_sound_direction: int = determine_direction(
            (sender_x, sender_y), (self.x, self.y), (self.known_map.width, self.known_map.height))

        sound_direction -= 1
        sound_direction: int = predicted_sound_direction - sound_direction

        if sound_direction < 0:
            sound_direction = sound_direction + 8

        if sound_direction % 2 != 0:
            raise ValueError("The math is not mathing: ")

        self.direction = TrantorianDirection(sound_direction / 2)

        return True


    def drop_stuff(self) -> None: # TODO drop only what's needed for levelup
        """drops all our stuff
        """
        for obj, val in self.inventory.items():
            if obj == 'food':
                continue
            for _ in range(val):
                self.drop_object(obj)
        return

##################################  COMMANDS  ##################################



    def forward(self) -> bool:
        """try to go forward, update the position if success
        time limit : 7/f

        Returns:
            bool: true if it worked, otherwise false
        """
        if self.dead:
            return False
        self.client.send_cmd("Forward")
        answer = self.wait_answer()
        if answer != 'ok':
            return False
        x, y = self.direction.get_offset()
        # TODO this cause probleme in the map serialization
        # as the update time is not reset, we came to a point where player = -1
        # if we update the last_time the objects will be wrong
        # self.get_current_case().content["player"] -= 1
        self.x += x
        self.y += y
        self.x %= self.client.size_x
        self.y %= self.client.size_y
        # self.get_current_case().content["player"] += 1
        return True

    def right(self) -> bool:
        """try to turn right, update the position if success
        time limit : 7/f

        Returns:
            bool: true if it worked, otherwise false
        """
        if self.dead:
            return False
        self.client.send_cmd("Right")
        answer = self.wait_answer()
        if answer != 'ok':
            return False
        self.direction = TrantorianDirection((self.direction - 1) % 4)
        return True

    def left(self) -> bool:
        """try to turn left, update the position if success
        time limit : 7/f

        Returns:
            bool: true if it worked, otherwise false
        """
        if self.dead:
            return False
        self.client.send_cmd("Left")
        answer = self.wait_answer()
        if answer != 'ok':
            return False
        self.direction = TrantorianDirection((self.direction + 1) % 4)
        return True

    def look_around(self) -> bool:
        """try to look around
        time limit : 7/f

        Returns:
            bool: false if parsing failed
        """
        if self.dead:
            return False
        self.client.send_cmd("Look")

        cases = split_list(self.wait_answer())
        if cases == []:
            return False
        nb_case: int = len(cases)
        i: int = 1
        current = 1
        while i < 9 and current < nb_case:
            current += 1 + 2 * i
            i += 1
        if nb_case != current:
            return False
        self.level = i - 1

        self.get_current_case().fill_from_str(cases.pop(0))
        direct: int = -1
        if self.direction in [TrantorianDirection.DOWN, TrantorianDirection.LEFT]:
            direct = 1
        if self.direction in [TrantorianDirection.UP, TrantorianDirection.DOWN]:
            for i in range(1, self.level + 1):
                yc = (self.y + i * direct) % self.client.size_y
                for x in range(self.x - i, self.x + 1 + i):
                    self.known_map.tiles[yc][x % self.client.size_x].fill_from_str(cases.pop(0))
            return True
        for i in range(1, self.level + 1):
            xc = (self.x + i * direct) % self.client.size_x
            for y in range(self.y - i, self.y + i + 1):
                self.known_map.tiles[y % self.client.size_y][xc].fill_from_str(cases.pop(0))
        return True

    def get_inventory(self) -> bool:
        """update the inventory
        time limit : 1/f

        Returns:
            bool: false if parsing failed
        """
        if self.dead:
            return False
        self.client.send_cmd("Inventory")

        answer = self.wait_answer()
        if self.dead:
            return False
        content = split_list(answer)
        if content == []:
            print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
            return False
        for e in content:
            key, val = e.split()
            if key not in self.inventory:
                return False
            try:
                self.inventory[key] = int(val)
            except ValueError:
                return False
        return True

    def broadcast(self, content: str, receivers: list[str]) -> bool: # TODO remove the map
        """broadcast a message
        time limit : 7/f
        "sender_uuid$receiver_uuid$type$content$infos$map"

        Args:
            content (str): message content
            receivers (list[str]): list of receivers (["all"] for all, or [] for troll)

        Returns:
            bool: true if ok, otherwise false
        """
        if self.dead:
            return False
        if receivers == []:
            msg = self.received_messages.pop(0)
            self.client.send_cmd("Broadcast " + msg)
            return self.wait_answer() == 'ok'
        msg = f'{self.uid}$'
        msg += '|'.join(receivers) + '$'
        msg += f'{content}$'
        msg += f'{pack_infos(
            self.others, self.uid, self.inventory, self.level, self.x, self.y, self.dispo_incant)}'
        self.client.send_cmd("Broadcast " + msg)
        return self.wait_answer() == 'ok'

    def get_unused_slot(self) -> bool:
        """update the unused_slot
        time limit : instant

        Returns:
            bool: false if parsing failed
        """
        if self.dead:
            return False
        self.client.send_cmd("Connect_nbr")
        answer = self.wait_answer()
        try:
            self.unused_slot = int(answer)
        except ValueError:
            return False
        return True

    def asexual_multiplication(self, queue: Queue) -> bool:
        """selffucking for a new trantorian
        time limit : 42/f

        Args:
            queue (Queue): Queue to put in the good hole to fertilize itself

        Returns:
            bool: false if self is sterile
        """
        if self.dead:
            return False
        self.client.send_cmd("Fork")
        if self.wait_answer() != 'ok':
            return False
        queue.put("birth")
        self.egg_pos = (self.x, self.y)
        return True

    def eject(self) -> bool:
        """eject trantorians on the same case
        time limit : 7/f

        Returns:
            bool: false if self is weak
        """
        if self.dead:
            return False
        self.client.send_cmd("Eject")
        return self.wait_answer() == 'ok'

    def take_object(self, obj: str) -> bool:
        """take object on the same case
        time limit : 7/f

        Args:
            obj (str): obj to take

        Returns:
            bool: true if the object is took
        """
        if self.dead:
            return False
        self.client.send_cmd("Take " + obj)
        if self.wait_answer() != 'ok':
            return False
        self.inventory[obj] += 1
        return True

    def drop_object(self, obj: str) -> bool:
        """drop object on the same case
        time limit : 7/f

        Args:
            obj (str): obj to drop

        Returns:
            bool: true if the object is took
        """
        if self.dead:
            return False
        self.client.send_cmd("Set " + obj)
        ans = self.wait_answer()
        if ans != 'ok':
            return False
        if obj not in self.inventory:
            return False
        self.inventory[obj] -= 1
        return True

    def start_incantation(self) -> bool: # TODO check what happend for player on the same tile
        """Start the incantation
        time limit : 300/f

        Returns:
            bool: true if the incantation did work
        """
        if self.dead:
            return False
        self.client.send_cmd("Incantation")
        answer = self.wait_answer()
        if answer != "Elevation underway":
            print(self.uid, "elev failed", answer)
            return False
        answer = self.wait_answer()
        if len(answer) < 16 or answer[:15] != "Current level: " or answer[15] not in "12345678":
            print("level failed")
            return False
        self.level = int(answer[15])
        return True

if __name__ == "__main__":
    trantorien = Trantorian("localhost", 8000, "dan")
    trantorien.update_direction_from_msg(5, 5, 6)
    print(trantorien.direction)
