import os
import errno
import struct


FIFO = 'CtoPy'
mensajes = []
mensaje = ''

try:
    os.mkfifo(FIFO)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured")

with open(FIFO, "rb") as fifo:
    print("FIFO opened")
    while True:
        data = fifo.read(1)

        if len(data) == 0:
            print("Writer closed")
            break

        mensaje += struct.unpack("=s", data)[0].decode("utf-8")
        if len(mensaje) == 11:
            print(mensaje)
            mensaje = ''
