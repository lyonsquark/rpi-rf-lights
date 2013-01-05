#!/usr/bin/env python

# Transmit a command

import sys
import serial
import lockfile
#theLockFile = '/home/pi/homeControl/lockFile'
theLockFile = '/run/lock/homeControlLock'

import logfile as l

def send(command):

    if command == 't':
        l.log.debug("Sending code %s", command)
    else:
        l.log.info("Sending code %s", command)

    # Acquire a lock on the arduino
    l.log.debug("Awaiting lock")
    lock = lockfile.FileLock(theLockFile)
    with lock:

        # Open connection to Arduino
        ser = serial.Serial("/dev/ttyUSB0", 9600, timeout=3)

        # Waiting for ready?
        l.log.debug("Waiting for ready")
        r = ser.readline()
	if r:
            l.log.debug("Got %s", r.strip())

        # Send the command
        l.log.debug("Sending command %s", command)
        ser.write(command)
        l.log.debug("Done sending, awaiting reply")

        response = ''

        # Get any response?
        while 1:
            r = ser.readline()
            if not r:
                l.log.error("Timeed out waiting for Ack")
                ser.close()
                return "Error"
   
            r = r.strip()
            l.log.debug("intermediate response is %s", r)
            if not 'Ack' in r:
                if r == '':
                    l.log.error("Timed out waiting for Ack")
                    ser.close()
                    return "Error"
                elif "ready" in r.lower():
		    continue
                else:
                    response += r + "\n"
            else:
                break

        ser.close()

    l.log.debug("response is %s", response.strip())


    return response

if __name__ == '__main__':
    print send(sys.argv[1]).strip()
