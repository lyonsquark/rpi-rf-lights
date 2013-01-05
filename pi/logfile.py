# Common log file

import logging

theLogFile = "/home/pi/homeControl/logFile"
logging.basicConfig(filename=theLogFile,
    format='%(asctime)s %(levelname)s:%(message)s',
    level=logging.INFO)

log = logging
