from trentorian.trentorian import Trantorian
from parser.virtual.parser import IParser
from utils import determine_direction
from parser.concrete.message_type_parser import MessageType

from warnings import warn
from enum import IntEnum


class MessageRitualParser(IParser):
    """class for message of type becon parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a birth info message

        The message returned should ressemble something like that:
            "trentorianUid"
        """
        return "None"

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for birth info
        """
        if trentorian.state == "beacon":
            trentorian.number_of_ritual_ready += 1

        return trentorian
