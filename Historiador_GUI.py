#python -m PyQt5.uic.pyuic -x GUI.ui -o Interfaz.py --> to generate the python code from the ui
#pyrcc5 resource.qrc -o resource_rc.py --> to generate the _rc.py from the .qrc file to use images

#************************************* GUI ************************************#
from Interfaz import *
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import pyqtSlot, Qt, QPoint
from PyQt5.QtGui import QFont, QEnterEvent, QPixmap
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QDialog
from PyQt5 import QtCore
import Style

#********************************* Historiador ********************************#
import os
import sys
import time
import errno
import struct
import re
import threading
from random import randint


FIFO = 'CtoPy'
FIFO2 = 'PytoC'
mensaje = ''

'''
try:
    os.pipe(FIFO) #os.mkfifo(FIFO)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured1")
try:
    os.mkfifo(FIFO2)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured2")
'''

#************************************* APP ************************************#
class APP (QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__ (self):
        super().__init__()
        self.setupUi(self)
        
        historiador = threading.Thread(daemon=True,target=PIPES)
        historiador.start()

        self.x1 = list(range(100))  # 100 time points
        self.y1 = [randint(0,100) for _ in range(100)]  # 100 data points
        pen1 = pg.mkPen(color=(255, 0, 0))
        self.data_line =  self.widget.plot(self.x1, self.y1, pen=pen1, name = 'Prueba 1')
        self.timer = QtCore.QTimer()
        self.timer.setInterval(50)
        self.timer.timeout.connect(self.update_plot_data)
        self.timer.start()

    def update_plot_data(self):
        self.x1 = self.x1[1:]  # Remove the first y element.
        self.x1.append(self.x1[-1] + 1)  # Add a new value 1 higher than the last.
        self.y1 = self.y1[1:]  # Remove the first
        self.y1.append(randint(0,100))  # Add a new random value.
        self.data_line.setData(self.x1, self.y1)  # Update the data.

    def presionado1(self):
        print(hola)

def PIPES():
    global ventanamain
    with open(FIFO, "rb") as fifo:
        print("FIFO opened")
        while True:
            data = fifo.read(1)

            if len(data) == 0:
                print("Writer closed")
                break

            mensaje += struct.unpack("=s", data)[0].decode("utf-8")
            if '\n' in mensaje:
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

                print(hora)
                mensaje = ''
    return

app = QtWidgets.QApplication([])
stylesheet = Style.StyleSheets("dark_orange")
app.setStyleSheet(stylesheet)
ventanamain=APP()
ventanamain.show()
app.exec_()
