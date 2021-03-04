import logging
from logging.handlers import TimedRotatingFileHandler


def get_logger(logger_name):
    logger = logging.getLogger(logger_name)
    # logger.setLevel(logging.DEBUG)
    fh = TimedRotatingFileHandler('test_log.log', when='D')
    fh.setLevel(logging.DEBUG)

    # create formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    logger.addHandler(fh)
    return logger
