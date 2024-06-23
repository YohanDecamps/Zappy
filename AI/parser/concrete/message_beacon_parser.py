from trentorian import Trantorian
from parser.virtual import IParser
from utils import determine_direction
from .message_type_parser import MessageType
from time import time
from warnings import warn
from enum import IntEnum

class ParamsType(IntEnum):
    """Sound direction enum
    """
    UID = 0


class MessageBeaconParser(IParser):
    """class for message of type becon parsing
    """

    def serialize(self, trentorian: Trantorian) -> str:
        """Return the message string for creting a birth info message

        The message returned should ressemble something like that:
            "trentorianUid"
        """
        return f"{trentorian.uid}_{time()}"

    def deserialize(self, trentorian: Trantorian, message_content: str, message_hitpoint: int) -> Trantorian:
        """Deserialize the message content, for birth info
        """
        try:
            content, last_time = message_content.split('_')
            last_time = float(last_time)
        except ValueError:
            return

        if trentorian.state == "beacon":
            try:
                if float(trentorian.uid) < float(content):
                    return trentorian
            except ValueError:
                warn(f"Invalid message content for beacon message: {message_content}")
                return trentorian


        trentorian.state = "going somewhere"
        trentorian.last_beacon_uid = content
        trentorian.last_beacon_direction = message_hitpoint
        trentorian.last_beacon_time = last_time

        return trentorian
