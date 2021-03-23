import os
import sys
# import platform
import logging
from concurrent.futures.thread import ThreadPoolExecutor
from logging.handlers import TimedRotatingFileHandler
# import aiologger
#
# from aiologger.handlers.files import (
#     AsyncFileHandler,
#     BaseAsyncRotatingFileHandler,
#     AsyncTimedRotatingFileHandler,
#     RolloverInterval,
#     ONE_WEEK_IN_SECONDS,
#     ONE_DAY_IN_SECONDS,
#     ONE_MINUTE_IN_SECONDS,
#     ONE_HOUR_IN_SECONDS,
# )
#
#
# from aiologger.utils import get_running_loop
# from aiologger.filters import StdoutFilter
# from aiologger.handlers.streams import AsyncStreamHandler
# from aiologger.levels import LogLevel
# from aiologger.logger import Logger
# from aiologger.records import LogRecord
# # from tests.utils import make_read_pipe_stream_reader
# from aiologger.formatters.base import Formatter
import inspect

# from common import gr

_log_tp_executor = ThreadPoolExecutor(max_workers=1)
# _th_executor.submit()


class LogManager:
    log_tag = ""
    log_path = ""
    file_handler = None

    @staticmethod
    def set_log_tag(tag):
        LogManager.log_tag = tag

    @staticmethod
    def set_log_path(path):
        LogManager.log_path = path

    @staticmethod
    def get_logger(logger_name=None):
        if logger_name is None:
            try:
                caller_class = inspect.stack()[1][0].f_locals["self"].__class__.__name__
                logger_name = caller_class
            except:
                raise Exception("logger_name is None and caller class error")
        if LogManager.file_handler is None:
            if LogManager.log_tag == "":
                raise Exception("LogManager Error: log tag is empty!")
            LogManager.file_handler = TimedRotatingFileHandler(
                LogManager.log_path + LogManager.log_tag + ".log", when="D")
        return AsyncLogger(logger_name, LogManager.file_handler)
        # _temp_file_name = 'test_log.log'
        #
        # use_st_logger = True
        # # _th_executor = ThreadPoolExecutor(max_workers=1)
        # # _th_executor.submit()
        # # if platform.system() == 'Linux':
        # if use_st_logger:
        #     # logger = logging.getLogger(logger_name)
        #     # # logger.setLevel(logging.DEBUG)
        #     # fh = TimedRotatingFileHandler('test_log.log', when='D')
        #     # # fh = TimedRotatingFileHandler(_temp_file_name, when='S')
        #     # # fh.setLevel(logging.DEBUG)
        #     #
        #     # formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
        #     # fh.setFormatter(formatter)
        #     # logger.addHandler(fh)
        #     # # fh.setLevel(logging.DEBUG)
        #     # logger.setLevel(logging.DEBUG)
        #     # # logger.shu
        #
        #     return AsyncLogger(logger_name)
        # # else:
        # #     logger = aiologger.logger.Logger()
        # #     handler = AsyncTimedRotatingFileHandler(
        # #         # filename=self.temp_file.name,
        # #         filename=_temp_file_name,
        # #         when=RolloverInterval.SECONDS,
        # #
        # #         # backup_count=1,
        # #     )
        # #     # handler.stream
        # #     formatter = Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
        # #     handler.formatter = formatter
        # #     logger.add_handler(handler)
        # #     logger.level = LogLevel.INFO
        # #
        # #     # if __name__ == '__main__':
        # #     #
        # #     #     # async def main():
        # #     #     #     _logger = LogManager.get_logger('test_logger')
        # #     #     #
        # #     #     # # 'application' code
        # #     #     #     await _logger.debug('debug message')
        # #     #     #     await _logger.info('info message')
        # #     #     #     await _logger.warning('warn message')
        # #     #     #     await _logger.error('error message')
        # #     #     #     await _logger.critical('critical message')
        # #     #     #     # await _logger.shutdown()
        # #     #     #
        # #     #     # import asyncio
        # #     #     #
        # #     #     # loop = asyncio.get_event_loop()
        # #     #     # loop.run_until_complete(main())
        # #     #     # loop.close()
        # #
        # #     return logger


class AsyncLogger:

    def __init__(self, logger_name, fh):
        # logging.basicConfig(
        #     format='%(asctime)s - %(name)s - %(levelname)s : %(message)s',
        #     handlers=[TimedRotatingFileHandler(
        #         LogManager.log_path + LogManager.log_tag + ".log", when='S')],
        # )
        _logger = logging.getLogger(logger_name)

        # fh = TimedRotatingFileHandler(
        #     LogManager.log_path + LogManager.log_tag + ".log", when='D')
        # fh = TimedRotatingFileHandler(_temp_file_name, when='S')
        # fh.setLevel(logging.DEBUG)
        # formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s : %(message)s')
        fh.setFormatter(formatter)
        _logger.addHandler(fh)
        # fh.setLevel(logging.DEBUG)
        _logger.setLevel(logging.DEBUG)

        self._logger = _logger

    @staticmethod
    def join_caller_filename_lineno(msg):
        # caller = inspect.getframeinfo(inspect.stack()[2][0])
        caller = inspect.stack()[2]
        # return ''.join(('[', inspect.stack()[2].filename, ':', str(inspect.stack()[2].lineno), ']', msg))
        return ''.join(('[', os.path.basename(caller.filename), ':', str(caller.lineno), ']: ', msg))

    def debug(self, msg, *args, **kwargs):
        _log_tp_executor.submit(self._logger.debug, self.join_caller_filename_lineno(msg), *args, **kwargs)

    def info(self, msg, *args, **kwargs):
        _log_tp_executor.submit(self._logger.info, self.join_caller_filename_lineno(msg), *args, **kwargs)

    def warning(self, msg, *args, **kwargs):
        _log_tp_executor.submit(self._logger.warning, self.join_caller_filename_lineno(msg), *args, **kwargs)

    def error(self, msg, *args, **kwargs):
        _log_tp_executor.submit(self._logger.error, self.join_caller_filename_lineno(msg), *args, **kwargs)

    def critical(self, msg, *args, **kwargs):
        _log_tp_executor.submit(self._logger.critical, self.join_caller_filename_lineno(msg), *args, **kwargs)

    def log_last_except(self):
        tp, value, traceback = sys.exc_info()
        tb_cont = convert_python_tb_to_str(tp, value, traceback)
        _log_tp_executor.submit(
            self._logger.error,
            self.join_caller_filename_lineno(f'on_traceback, type:{tp}, value:{value}, traceback:{tb_cont}'))


def convert_python_tb_to_str(t, v, tb, limit=None):
    tbinfo = [str(t), str(v)]
    if tb is None:
        return
    n = 0
    while tb and (limit is None or n < limit):
        frame = tb.tb_frame
        # 上传服务器的文件名筛选
        # script_idx = frame.f_code.co_filename.find('script', 0)
        # if script_idx != -1:
        #	filename = frame.f_code.co_filename[script_idx:]

        MAX_TRACE_LEN = 1024
        try:
            locals = dict(frame.f_locals)
            local_self = locals.pop('self', None)
            if local_self:
                local_str = "{'self': " + str(local_self) + ", " + str(locals)[1:]
            else:
                local_str = str(locals)
        except:
            local_str = 'Cannot print locals'
        if len(local_str) > MAX_TRACE_LEN:
            local_str = local_str[:MAX_TRACE_LEN] + '...'

        line = [
            '  File "%s"' % frame.f_code.co_filename,
            'line %d' % tb.tb_lineno,
            'in %s' % frame.f_code.co_name,
            ]
        tbinfo.append(', '.join(line))
        tbinfo.append('    locals=' + local_str)
        tb = tb.tb_next
        n = n + 1
    return '\n'.join(tbinfo)


if __name__ == '__main__':

    # async def main():
    #     _logger = LogManager.get_logger('test_logger')
    #
    # # 'application' code
    #     await _logger.debug('debug message')
    #     await _logger.info('info message')
    #     await _logger.warning('warn message')
    #     await _logger.error('error message')
    #     await _logger.critical('critical message')
    #     # await _logger.shutdown()
    #
    # import asyncio
    #
    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(main())
    # loop.close()

    # _logger.info('siguoyinie')
    # _logger.debug('siguoyinie')

    # create logger
    # logger = logging.getLogger('simple_example')
    
    #     # logger.setLevel(logging.DEBUG)
    # fh = TimedRotatingFileHandler('logmm.log', when='D')
    # # fh = TimedRotatingFileHandler(_temp_file_name, when='S')
    # fh.setLevel(logging.DEBUG)

    # formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
    # fh.setFormatter(formatter)
    # logger.addHandler(fh)

    # # logger.setLevel(logging.DEBUG)

    # # # create console handler and set level to debug
    # # ch = logging.StreamHandler()
    # # ch.setLevel(logging.DEBUG)

    # # # create formatter
    # # formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # # # add formatter to ch
    # # ch.setFormatter(formatter)

    # # # add ch to logger
    # # logger.addHandler(ch)

    # 'application' code
    LogManager.set_log_tag('test_log')
    logger = LogManager.get_logger('test_log')
    logger.debug('debug message')
    logger.info('info message')
    logger.warning('warn message')
    logger.error('error message')
    logger.critical('critical message')

    try:
        1/0
    except:
        # import sys
        # tp, value, traceback = sys.exc_info()
        #
        # tb_cont = convert_python_tb_to_str(tp, value, traceback)
        # logger.error(f'on_traceback, type:{tp}, value:{value}, traceback:{tb_cont}')
        # logging.exception("Deliberate divide by zero traceback")
        logger.log_last_except()
