import os
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


_tp_executor = ThreadPoolExecutor(max_workers=1)
# _th_executor.submit()


class AsyncLogger:

    def __init__(self, logger_name):
        logger = logging.getLogger(logger_name)
        # logger.setLevel(logging.DEBUG)
        fh = TimedRotatingFileHandler('test_log.log', when='D')
        # fh = TimedRotatingFileHandler(_temp_file_name, when='S')
        # fh.setLevel(logging.DEBUG)
        # formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s : %(message)s')
        fh.setFormatter(formatter)
        logger.addHandler(fh)
        # fh.setLevel(logging.DEBUG)
        logger.setLevel(logging.DEBUG)
        # logger.shu
        # return logger
        self._logger = logger

    @staticmethod
    def join_caller_filename_lineno(msg):
        # caller = inspect.getframeinfo(inspect.stack()[2][0])
        caller = inspect.stack()[2]
        # return ''.join(('[', inspect.stack()[2].filename, ':', str(inspect.stack()[2].lineno), ']', msg))
        return ''.join(('[', os.path.basename(caller.filename), ':', str(caller.lineno), ']: ', msg))

    def debug(self, msg):
        _tp_executor.submit(self._logger.debug, self.join_caller_filename_lineno(msg))

    def info(self, msg):
        _tp_executor.submit(self._logger.info, self.join_caller_filename_lineno(msg))

    def warning(self, msg):
        _tp_executor.submit(self._logger.warning, self.join_caller_filename_lineno(msg))

    def error(self, msg):
        _tp_executor.submit(self._logger.error, self.join_caller_filename_lineno(msg))

    def critical(self, msg):
        _tp_executor.submit(self._logger.critical, self.join_caller_filename_lineno(msg))


class LogManager:

    # def
    @staticmethod
    def get_logger(logger_name):
        return AsyncLogger(logger_name)
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
    logger = LogManager.get_logger('test_log')
    logger.debug('debug message')
    logger.info('info message')
    logger.warning('warn message')
    logger.error('error message')
    logger.critical('critical message')