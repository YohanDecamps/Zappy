from trentorian import Trantorian
from parser.virtual import IParser
from utils import determine_direction
from .message_type_parser import MessageType

from warnings import warn
from enum import IntEnum

class ParamsType(IntEnum):
    """Sound direction enum
    """
    SENDERX = 0
    SENDERY = 1
    REICEIVERX = 2
    REICEIVERY = 3
    TEAM_SIZE = 4


class MessageBirthInfoParser(IParser):
    """class for message of type born parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a birth info message

        The message returned should ressemble something like that:
            "senderx_sendery_reiceiverx_reiceivery_teamSize"
        """
        return f'{trentorian.egg_pos[ParamsType.SENDERX]}_{
            trentorian.egg_pos[ParamsType.SENDERY]}_{
                trentorian.x}_{
                    trentorian.y}_{
                            trentorian.team_size}'

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for birth info
        """
        params = message_content.split("_")

        if len(params) != 4:
            warn(f"Invalid message content: {message_content}")
            return trentorian

        trentorian.team_size = params[ParamsType.TEAM_SIZE]

        determine_direction((params[ParamsType.SENDERX], params[ParamsType.SENDERY]), (
            params[ParamsType.REICEIVERX], params[ParamsType.REICEIVERY]), (
                trentorian.known_map.width, trentorian.known_map.height))

        trentorian.x = params[ParamsType.REICEIVERX]
        trentorian.y = params[ParamsType.REICEIVERY]
        self.state = "adult"

        return trentorian

class MessageAskBirthParser(IParser):
    """class for message of type born parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a ask birth message

        The message returned should ressemble something like that:
            ""
        """
        return ""

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for ask birth
        """
        if not trentorian.state == "wait birth":
            return trentorian

        trentorian.state = "ask_birth"

        return trentorian
