import datetime
import logging
import os

from COMP5047_Backend.Tools.Logger import Logger

try:
    logDirectory = 'logs'
    print('| INFO | Logger starting ...')
    if not os.path.exists(logDirectory):
        os.makedirs(logDirectory)
    logging.basicConfig(level=logging.DEBUG, filename=logDirectory + '/log' + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + '.log')
except Exception as e:
    Logger.error(e)
