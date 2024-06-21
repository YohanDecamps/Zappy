#!/usr/bin/env python3
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
MAX_PLAYER = 12
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

class OtherIndex(IntEnum):
    """Other index enum
    """
    INV = 0
    LVL = 1
    X = 2
    Y = 3
    LAST_UPDATE = 4

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
        self.last_beacon_direction: int = 0
        self.last_beacon_uid: str = None
        self.last_beacon_time: float = 0
        self.number_of_ritual_ready: int = 0
        # least amount of food needed to go throught the longest distance and do an incantation
        self.mini_food: int = 20 + ((self.known_map.width + self.known_map.height) * 8 / FOOD)
        self.consider_dead: float = (self.known_map.width + self.known_map.height) * 8 + 400
        self.ticks: int = 0
        self.nbr_tests_ticks: int = 0
        self.tick_time: float = 0

    def born(self, queue: Queue):
        """launch a trantorian and do its life

        Args:
            queue (Queue): process shared queu (for birth)
        """
        try:
            self.dprint("live")
            self.start_living(queue)
            if self.dead:
                return
            self.first_level()
            while self.iter_food():
                can_level_up = self.wander(queue)
                success = False

                if self.state == "shaman":
                    self.dprint("start incant with:", can_level_up)
                    success = self.be_the_shaman(can_level_up)
                    if not success:
                        self.broadcast(MessageTypeParser().serialize(
                            MessageType.RITUAL_FAILED, self), can_level_up)

                if self.state == 'going somewhere':
                    success = self.follow_the_leader()

                if success:
                    success = self.wait_incant()
                self.dprint("incantation end", success)
                self.dispo_incant = True
                self.broadcast("just$update", ["all"])
        except BrokenPipeError:
            self.dprint("Server closed socket")
            self.dead = True
        self.dprint("died")
        return

    def be_the_shaman(self, can_do_incant: list) -> bool:
        """call the others and do the incantation process

        Args:
            can_do_incant (list): trants to do the incantation with

        Returns:
            bool: incantation's success
        """
        self.dispo_incant = False
        self.beacon(can_do_incant)
        if self.state != "ready": # in case an other one calls him
            return False
        self.drop_stuff()
        return self.start_incantation()


    def follow_the_leader(self) -> bool:
        """go and do the incantation whit the one who called

        Returns:
            bool: incantation's succes
        """
        self.dprint('Follow leader', self.last_beacon_uid)
        self.dispo_incant = False
        self.broadcast("just$update", ["all"])
        if not self.follow_beacon():
            self.dprint("didnt go to beacon")
            return False
        self.drop_stuff()
        self.broadcast(MessageTypeParser().serialize(
            MessageType.RITUAL_READY, self), [self.last_beacon_uid])
        return True

    def first_level(self) -> None:
        """script to fastly go to level 2 and acquire some food
        """
        ready: bool = False
        while self.iter_food() and not ready:
            self.look_around()
            if self.get_current_case().content["linemate"] > 0:
                break
            self.forward()
            direct: int = random.randint(0, 5)
            if direct == 1:
                self.left()
            elif direct == 2:
                self.right()
            ready = check_levelup(self.inventory, 2, {})
        if self.dead:
            return
        if self.start_incantation() is False:
            self.first_level()
        if self.wait_incant() is False:
            self.first_level()
        return

##############################  UTILS  #######################################


    def dprint(self, *args, **kwargs):
        """a print with the uid
        """
        print(self.uid[-6:], ':', *args, **kwargs)

    def iter(self) -> bool:
        """update the needed values and check if the next iteration is possible (not dead)

        Returns:
            bool: can do the next iteration
        """
        if self.dead:
            return False
        if self.ticks > self.consider_dead - 20:
            self.broadcast('just$update', ["all"])
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

    def wander(self, queue: Queue) -> list[str]:
        """look and take stuff until we can do an incantation, or we are called

        Args:
            queue (Queue): queue to birth other trantorians

        Returns:
            list[str]: list of players to do the incantation
        """
        self.state = "wander"
        can_level_up = []
        i = 0
        while self.iter_food() and self.state == "wander" and not can_level_up:
            self.get_unused_slot()
            if len(self.others) < 7 and self.unused_slot > 0:
                self.asexual_multiplication(queue)
            self.look_around()
            for _ in range(self.level): # TODO maybe some priority order here
                if not self.take_tile_objects():
                    self.forward()
                    break
                self.forward()
            direct: int = random.randint(0, 5)
            if direct == 1:
                self.left()
            elif direct == 2:
                self.right()
            can_level_up = get_incantation_team(self.inventory, self.level + 1, self.others)
            i += 1
            if not i % 4:
                continue
            # self.troll_broadcasts() # TODO put this back
            self.broadcast("just$update", ["all"])
            self.get_unused_slot()
            if len(self.others) < MAX_PLAYER and self.unused_slot > 0:
                self.asexual_multiplication(queue)
        if can_level_up and self.state != "going somewhere":
            self.state = "shaman"
        return can_level_up

    def follow_beacon(self) -> bool:
        """follow the beacon to make a ritual

        Returns:
            bool: true if we are on the same case at the end
        """
        while self.iter() and self.state == "going somewhere":
            if self.last_beacon_direction == 0:
                self.state = "ritual_prep"
                self.dprint(" I am ready with", self.last_beacon_uid)
                break

            if self.last_beacon_direction < 5 and self.last_beacon_direction != 1:
                self.left()
            elif self.last_beacon_direction == 5:
                self.left()
                self.left()
            elif self.last_beacon_direction > 5:
                self.right()

            self.forward()
            self.state = "wait"
            while not self.dead and self.state == "wait":
                self.get_answer()
        return not self.dead and self.state == "ritual_prep"

    def beacon(self, receivers: list) -> None:
        """set ourselves as a beacon to atract the others

        Args:
            receivers (list): people to regroup
        """
        self.state = "beacon"
        self.number_of_ritual_ready = 0
        while self.iter() and self.state == "beacon":
            self.kill_others()
            if not all(e in self.others for e in receivers):
                self.state = "wander"
                return
            self.broadcast(MessageTypeParser().serialize(MessageType.BEACON, self), receivers)
            if self.number_of_ritual_ready >= LEVELS[self.level + 1][0] - 1:
                self.dprint("we are ready", receivers)
                self.state = "ready"
        return

    def start_living(self, queue: Queue) -> None:
        """ First step of the life (reproduction)

        Args:
            queue (Queue): queue to birth other trantorians
        """
        if not self.iter_food():
            return
        if len(self.others) > 15:
            self.suicide()
            return
        if self.client.team_size > 0 and len(self.others) < MAX_PLAYER:
            self.asexual_multiplication(queue)
        self.broadcast("im$alive", ["all"])
        for _ in range(10): # use to set a first value for tick_time
            self.get_inventory()
        return

    def wait_incant(self) -> bool:
        """wait for the end of an incantation (level change or broadcast)

        Returns:
            bool: incantation failed or not
        """
        answer = ""
        while not self.dead and not answer.startswith("Current level: "): # TODO do that better
            answer = self.get_answer()
            if self.state == "wander":
                self.dprint("wander", time())#, answer)
                return False
            if answer == "incantation end":
                return False
        if self.dead or not answer.startswith("Current level: "):
            self.dprint("incant failed", answer)
            return False
        try:
            self.level = int(answer[15])
        except ValueError:
            self.dprint("incant failed", answer)
            return False
        self.broadcast(MessageTypeParser().serialize(MessageType.RITUAL_FINISH, self), "all")
        self.dprint("lvl:", self.level)
        return True

    def troll_broadcasts(self)-> None:
        """send weird broadcast to troll
        """
        tt = random.randint(0, 10)
        if len(self.received_messages) == 0:
            return
        if tt == 1:
            self.broadcast('.', [])
        elif tt == 2:
            idx = random.randint(0, len(self.received_messages))
            self.broadcast(self.received_messages.pop(idx), [])
        elif tt == 3:
            idx = random.randint(0, len(self.received_messages))
            idx2 = random.randint(0, len(self.received_messages[idx]))
            self.broadcast(self.received_messages[idx][:-(idx2 / 2)], [])
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
        content: dict = self.get_current_case().content
        if content["player"] > 1:
            return False
        for obj, quantity in content.items():
            if obj == "egg" or obj == "player":
                continue
            for _ in range(quantity):
                if self.take_object(obj):
                    return False
        return True

    def merge_others(self, received: dict) -> None:
        """merge a received list of user into the current one
        remove the ones that are to old (suspect died)

        Args:
            received (dict): received informations
        """
        dead_time: int = self.consider_dead * self.tick_time
        for uuid, infos in received.items():
            update = infos[4]
            if uuid not in self.others:
                self.others[uuid] = infos
            elif update > self.others[uuid][OtherIndex.LAST_UPDATE]:
                self.others[uuid] = infos
            diff = time() - self.others[uuid][OtherIndex.LAST_UPDATE]
            if diff > dead_time:
                self.others.pop(uuid)
        return

    def kill_others(self) -> None:
        """remove the others that are too old from the list
        """
        uids = list(self.others.keys())[::]
        for uid in uids:
            infos = self.others[uid]
            diff = time() - infos[4]
            if diff > self.consider_dead * self.tick_time:
                self.others.pop(uid)
        return

    def wait_answer(self) -> str: # TODO, receive "Current level" from an incantation ?"
        """wait for an answer, handle the message, eject and current level

        Returns:
            str: last server answer
        """
        answer: str = self.get_answer()
        while answer.startswith('message') or answer.startswith('eject') or answer.startswith("Current level: "):
            answer = self.get_answer()
        if answer.startswith("dead"):
            self.dead = True
        return answer

    def get_answer(self) -> str: # TODO, receive "Current level" from an incantation ?"
        """wait for an answer, handle the message and eject

        Returns:
            str: last server answer
        """
        answer: str = self.client.get_answer()
        if answer.startswith('message'):
            self.receive_message(answer)
        if answer.startswith('eject'):
            self.handle_eject(answer)
        if answer.startswith("dead") or answer == '':
            self.dead = True
        return answer

    def receive_message(self, msg: str) -> None:
        """handle the broadcast reception
            our messages are composed of :
            "sender_uuid$receiver_uuid$type$content$infos$map"

        Args:
            msg (str): message sent by the server
        """
        msg = msg[8:]
        direct, msg = msg.split(',')
        direct: int = int(direct)
        msg = msg.strip()
        if not msg.startswith(self.team):
            self.received_messages.append(msg)
            return
        msg = msg[len(self.team):]
        parts = msg.strip().split("$")
        if len(parts) != 5:
            return
        sender, receivers, msg_type, content, infos = parts
        info_unpacked = unpack_infos(infos, self.uid)
        self.merge_others(info_unpacked)
        if receivers != "all" and self.uid not in receivers.split('|'):
            return
        try:
            MessageTypeParser().deserialize(self, int(msg_type), content, direct)
        except (KeyError, ValueError):
            return
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
        self.x = (self.x - off_x) % self.client.size_x
        self.y = (self.y - off_y) % self.client.size_y
        return


    def drop_stuff(self) -> None:
        """drops all our stuff
        """
        needed = LEVELS[self.level + 1][1]
        idx: int = 0
        for obj, val in self.inventory.items():
            if obj == 'food':
                continue
            to_drop = min(val, needed[idx])
            for _ in range(to_drop):
                self.drop_object(obj)
            idx += 1
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
        self.ticks += 7
        self.client.send_cmd("Forward")
        answer = self.wait_answer()
        if answer != 'ok':
            return False
        x, y = self.direction.get_offset()
        self.x += x
        self.y += y
        self.x %= self.client.size_x
        self.y %= self.client.size_y
        return True

    def right(self) -> bool:
        """try to turn right, update the position if success
        time limit : 7/f

        Returns:
            bool: true if it worked, otherwise false
        """
        if self.dead:
            return False
        self.ticks += 7
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
        self.ticks += 7
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
        self.ticks += 7
        self.client.send_cmd("Look")

        cases = split_list(self.wait_answer())
        if cases == []:
            self.dprint('look parse failed')
            return False

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
        self.ticks += 1
        t1 = time()
        self.client.send_cmd("Inventory")
        answer = self.wait_answer()
        t2 = time()
        if self.dead:
            return False

        content = split_list(answer)
        if content == []:
            self.dprint('inventory parse faile')
            return False
        self.tick_time = ((self.tick_time * self.nbr_tests_ticks + (t2 - t1))
                        / (self.nbr_tests_ticks + 1))
        self.nbr_tests_ticks += 1
        for e in content:
            if e == '':
                continue
            key, val = e.split()
            if key not in self.inventory:
                return False
            try:
                self.inventory[key] = int(val)
            except ValueError:
                return False
        return True

    def broadcast(self, content: str, receivers: list[str]) -> bool:
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
        self.ticks = 0
        self.kill_others()
        if receivers == []:
            self.client.send_cmd("Broadcast " + content)
            return self.wait_answer() == 'ok'
        msg = f'{self.team}'
        msg += f'{self.uid}$'
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
        self.ticks += 7
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
        self.ticks += 7
        self.client.send_cmd("Fork")
        if self.wait_answer() != 'ok':
            return False
        queue.put("birth")
        return True

    def eject(self) -> bool:
        """eject trantorians on the same case
        time limit : 7/f

        Returns:
            bool: false if self is weak
        """
        if self.dead:
            return False
        self.ticks += 7
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
        self.ticks += 7
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
        self.ticks += 7
        self.client.send_cmd("Set " + obj)
        ans = self.wait_answer()
        if ans != 'ok':
            return False
        if obj not in self.inventory:
            return False
        self.inventory[obj] -= 1
        return True

    def start_incantation(self) -> bool:
        """Start the incantation
        time limit : 300/f

        Returns:
            bool: true if the incantation did work
        """
        if self.dead:
            return False
        self.ticks += 300
        self.client.send_cmd("Incantation")
        answer = self.wait_answer()
        if answer != "Elevation underway":
            self.dprint("elev failed", answer)
            return False
        return True

if __name__ == "__main__":
    trantorien = Trantorian("localhost", 8000, "dan")
    # trantorien.update_direction_from_msg(5, 5, 6)
    print(trantorien.direction)
