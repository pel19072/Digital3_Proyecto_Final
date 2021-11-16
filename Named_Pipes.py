import os
import sys
import time
import errno
import struct
import re

FIFO = 'CtoPy'
mensaje = ''

try:
    os.mkfifo(FIFO)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured")

with open(FIFO, "rb") as fifo:
    print("FIFO opened")
    while True:
        try:
            data = fifo.read(1)

            if len(data) == 0:
                print("Writer closed")
                break

            mensaje += struct.unpack("=s", data)[0].decode("utf-8")
            if '\n' in mensaje:
                display = mensaje.split(",")
                display[0] = display[0][-1]
                display[1] = time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.localtime(int(display[1])))
                print(display[1])
                mensaje = ''
        except:
            sys.exit("Hubo un error en la recepcion de datos\n")
