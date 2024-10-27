import json

from COMP5047_Backend.Tools.StringTool import StringTool


class ExtendedJsonEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, bytes):
            return StringTool.bytes_to_string(obj)
        return json.JSONEncoder.default(self, obj)
