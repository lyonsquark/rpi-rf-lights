#!/usr/bin/env python

# Send the home a command

import sys
import transmitter as t
import logfile as l

def sendCommand(outlet, off):

    outlets = "0ABC"

    # Determine the command
    code = outlets[int(outlet)]

    if off:
        code = code.lower()

    t.send(code)

if __name__ == '__main__':
    outlet = sys.argv[1]
    off = sys.argv[2] == "off"
    sendCommand(outlet, off)
    l.log.info("Executed %s %s" % (outlet, sys.argv[2]) )
