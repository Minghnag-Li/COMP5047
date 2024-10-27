import datetime
import logging
import traceback

from COMP5047_Backend.Tools.OnThreadExecutor import OnThreadExecutor
from COMP5047_Backend.Tools.StringTool import StringTool


class Logger:
    LEVEL_DEBUG = 'debug'
    LEVEL_INFO = 'info'
    LEVEL_WARN = 'warn'
    LEVEL_ERROR = 'error'
    LEVEL_FATAL = 'fatal'
    LEVEL_NONE = 'none'
    LEVELS = [LEVEL_DEBUG, LEVEL_INFO, LEVEL_WARN, LEVEL_ERROR, LEVEL_FATAL, LEVEL_NONE]

    @staticmethod
    def get_header(level=''):
        match level:
            case 'debug':
                header = '| DEBUG | '
            case 'info':
                header = '| INFO | '
            case 'warn':
                header = '| WARN | '
            case 'error':
                header = '| ERROR | '
            case 'fatal':
                header = '| FATAL | '
            case 'none':
                header = '| NONE | '
            case _:
                header = '| INFO | '
        return header + '\t' + datetime.datetime.now().strftime("%Y/%m/%d-%H:%M:%S") + '\t | '

    @staticmethod
    def _get_log(level='', alog=''):
        logs = Logger.get_header(level) + str(alog)
        print(logs)
        extra_logs = '\r\n' + traceback.format_exc() + 'Stacktrace: \r\n' + StringTool.list_to_string(
            traceback.format_stack()[::-1])
        match level:
            case 'debug':
                return logs
            case 'info':
                return logs
            case 'warn':
                return logs
            case 'error':
                return logs + extra_logs
            case 'fatal':
                return logs + extra_logs
            case 'none':
                return logs
            case _:
                return logs

    @staticmethod
    def _log(level='', alog=''):
        Logger._log2(level, Logger._get_log(level, alog))


    @staticmethod
    def _log2(level='', logs=''):
        try:
            match level:
                case 'debug':
                    logging.debug(logs)
                case 'info':
                    logging.info(logs)
                case 'warn':
                    logging.warning(logs)
                case 'error':
                    logging.error(logs)
                    print(logs)
                case 'fatal':
                    logging.fatal(logs)
                    print(logs)
                case 'none':
                    logging.info(logs)
                case _:
                    logging.info(logs)
        except Exception as e:
            print('| ERROR | Log failed: ' + str(e) + '\t\t\r\n' + traceback.format_exc())

    @staticmethod
    def log(log=''):
        Logger.info(log)

    @staticmethod
    def debug(log=''):
        Logger._log(Logger.LEVEL_DEBUG, log)

    @staticmethod
    def info(log=''):
        Logger._log(Logger.LEVEL_INFO, log)

    @staticmethod
    def warn(log=''):
        Logger._log(Logger.LEVEL_WARN, log)

    @staticmethod
    def error(log=''):
        Logger._log(Logger.LEVEL_ERROR, log)

    @staticmethod
    def fatal(log=''):
        Logger._log(Logger.LEVEL_FATAL, log)


class TLogger(Logger):
    __onThreadExecutor = OnThreadExecutor.get_instance('Logger')

    @staticmethod
    def __tlog(level='', log=''):
        try:
            logs = Logger._get_log(level, log)
            TLogger.__onThreadExecutor.submit(lambda: Logger._log2(level, logs))
        except Exception as e:
            Logger.error(str(e))

    @staticmethod
    def tlog(log=''):
        TLogger.__tlog(Logger.LEVEL_INFO, log)

    @staticmethod
    def tdebug(log=''):
        TLogger.__tlog(Logger.LEVEL_DEBUG, log)

    @staticmethod
    def tinfo(log=''):
        TLogger.__tlog(Logger.LEVEL_INFO, log)

    @staticmethod
    def twarn(log=''):
        TLogger.__tlog(Logger.LEVEL_WARN, log)

    @staticmethod
    def terror(log=''):
        TLogger.__tlog(Logger.LEVEL_ERROR, log)

    @staticmethod
    def tfatal(log=''):
        TLogger.__tlog(Logger.LEVEL_FATAL, log)

    @staticmethod
    def wait():
        TLogger.__onThreadExecutor.wait()

    @staticmethod
    def exit():
        TLogger.__onThreadExecutor.cancel()
