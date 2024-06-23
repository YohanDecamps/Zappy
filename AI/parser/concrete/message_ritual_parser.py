from trentorian import Trantorian
from parser.virtual import IParser
from utils import determine_direction
from .message_type_parser import MessageType

from warnings import warn
from enum import IntEnum


class MessageRitualParser(IParser):
    """class for message of type becon parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a birth info message

        The message returned should ressemble something like that:
            "None"
        """
        return "None"

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for birth info
        """
        if trentorian.state == "beacon":
            trentorian.number_of_ritual_ready += 1

        return trentorian

class MessageRitualFailedParser(IParser):
    """class for message of type ritual failed parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a ritual failed message

        The message returned should ressemble something like that:
            "None"
        """
        return "None"

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for ritual failed
        """

        if trentorian.state != "beacon":
            trentorian.state = "wander"

        return trentorian

class MessageRitualFinishParser(IParser):
    """class for message of type ritual finish parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a ritual finish message

        The message returned should ressemble something like that:
            "trentorianUid"
        """
        return trentorian.uid

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for ritual finish
        """
        temp_list = list(trentorian.others[message_content])
        temp_list[5] = False
        trentorian.others[message_content] = tuple(temp_list)
        return trentorian
