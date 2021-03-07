import platform
import logging
from logging.handlers import TimedRotatingFileHandler
import aiologger

from aiologger.handlers.files import (
    AsyncFileHandler,
    BaseAsyncRotatingFileHandler,
    AsyncTimedRotatingFileHandler,
    RolloverInterval,
    ONE_WEEK_IN_SECONDS,
    ONE_DAY_IN_SECONDS,
    ONE_MINUTE_IN_SECONDS,
    ONE_HOUR_IN_SECONDS,
)


from aiologger.utils import get_running_loop
from aiologger.filters import StdoutFilter
from aiologger.handlers.streams import AsyncStreamHandler
from aiologger.levels import LogLevel
from aiologger.logger import Logger
from aiologger.records import LogRecord
# from tests.utils import make_read_pipe_stream_reader
from aiologger.formatters.base import Formatter


def get_logger(logger_name):
    _temp_file_name = 'test_log.log'

    # if platform.system() == 'Linux':
    #     logger = logging.getLogger(logger_name)
    #     # logger.setLevel(logging.DEBUG)
    #     fh = TimedRotatingFileHandler('test_log.log', when='D')
    #     # fh = TimedRotatingFileHandler(_temp_file_name, when='S')
    #     # fh.setLevel(logging.DEBUG)

    #     formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
    #     fh.setFormatter(formatter)
    #     logger.addHandler(fh)
    #     # fh.setLevel(logging.DEBUG)
    #     logger.setLevel(logging.DEBUG)
    #     # logger.shu
    #     return logger
    # else:
    logger = aiologger.logger.Logger()
    handler = AsyncTimedRotatingFileHandler(
        # filename=self.temp_file.name,
        filename=_temp_file_name,
        when=RolloverInterval.SECONDS,

        # backup_count=1,
    )
    # handler.stream
    formatter = Formatter('%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] : %(message)s')
    handler.formatter = formatter
    logger.add_handler(handler)
    logger.level = LogLevel.INFO
    # logger.handlers = handler
    # logger

    return logger

    # r1 = make_log_record(msg="testing - initial")
        # await handler.emit(r1)
        # self.assertTrue(os.path.exists(self.temp_file.name))

        # await asyncio.sleep(1.1)

        # r2 = make_log_record(msg="testing - after delay")
        # await handler.emit(r2)
        # await handler.close()


if __name__ == '__main__':

    async def main():
        _logger = get_logger('test_logger')


    # 'application' code
        await _logger.debug('debug message')
        await _logger.info('info message')
        await _logger.warning('warn message')
        await _logger.error('error message')
        await _logger.critical('critical message')
        await _logger.shutdown()

    import asyncio

    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
    loop.close()

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

    # # 'application' code
    # logger.debug('debug message')
    # logger.info('info message')
    # logger.warning('warn message')
    # logger.error('error message')
    # logger.critical('critical message')