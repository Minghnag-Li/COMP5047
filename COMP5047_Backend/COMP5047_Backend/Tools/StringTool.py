class StringTool:
    @staticmethod
    def split_string_by_length(string, length):
        return [string[i:i + length] for i in range(0, len(string), length)]

    @staticmethod
    def bytes_to_string(bytes):
        return '[ ' + ', '.join(StringTool.split_string_by_length(bytes.hex(), 2)) + ' ]'

    @staticmethod
    def list_to_string(strings):
        return '[ ' + ', '.join(strings) + ' ]'
