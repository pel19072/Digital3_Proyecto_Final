import os
import sys
import time
import errno
import struct
import re

FIFO = 'CtoPy'
FIFO2 = 'PytoC'
mensaje = ''
RTU = 0
evento = 0
hora = 0.0
botones = [0,0]
switches = [0,0,0]
leds = [0,0]
adc = 0.0

try:
    os.mkfifo(FIFO)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured 1")
'''
try:
    os.mkfifo(FIFO2)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured 2")
'''
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
                try:
                    display = mensaje.split(",")

                    RTU = display[0][-1]
                    evento = display[1]
                    hora = str(time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime(float(display[2])))) + '.' + str(float(display[3])//1000)
                    botones[0] = display[4]
                    botones[1] = display[5]
                    switches[0] = display[6]
                    switches[1] = display[7]
                    switches[2] = display[8]
                    leds[0] = display[9]
                    leds[1] = display[10]
                    adc = display[11]

                    print("UTR:{}|EVENTO:{}|HORA:{}\n".format(RTU,evento,hora))
                    mensaje = ''
                except:
                    print("Dato recivido en Python: {}".format(mensaje))
                    mensaje = ''

        except:
            print("Hubo un error en la recepcion de datos\n")
            #sys.exit("Hubo un error en la recepcion de datos\n")
