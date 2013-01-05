#!/usr/bin/env python
#
#  Delay turning on or off a light
#
import random
import time
import homeCommand as h
import logfile as l
import sys

outlet = sys.argv[1]
off    = sys.argv[2] == "off"
delay  = int(sys.argv[3])

theDelay = random.random() * delay
l.log.info("Delaying %s %s for %s min" % (outlet, sys.argv[2], theDelay) )

time.sleep(theDelay*60.0)

h.sendCommand(outlet, off)
