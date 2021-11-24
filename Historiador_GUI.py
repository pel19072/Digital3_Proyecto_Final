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
RTU = 0
evento = [0,0]
hora = [0.0, 0.0]
botones = [[0,0], [0,0]]
switches = [[0,0,0], [0,0,0]]
leds = [[0,0], [0,0]]
leds_tx = [[0,0], [0,0]]
adc = [0.0, 0.0]

hora_tmp = ['','']
buffer_tmp = [[''],['']]


try:
    os.mkfifo(FIFO)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured1")
try:
    os.mkfifo(FIFO2)
except OSError as oe:
    if oe.errno != errno.EEXIST:
        print("Error occured2")


#************************************* APP ************************************#
class APP (QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__ (self):
        super().__init__()
        self.setupUi(self)

        self.pushButton.clicked.connect(self.presionado1)
        self.pushButton_2.clicked.connect(self.presionado2)
        self.pushButton_3.clicked.connect(self.presionado3)
        self.pushButton_4.clicked.connect(self.presionado4)

        rx_hist = threading.Thread(daemon=True,target=PIPES_RX)
        rx_hist.start()

        tx_hist = threading.Thread(daemon=True,target=PIPES_TX)
        tx_hist.start()

        self.x1 = list(range(10))  # 100 time points
        self.y1 = [3.3 for _ in range(10)]  # 100 data points
        self.y2 = [3.3 for _ in range(10)]  # 100 data points
        self.data_line =  self.widget.addPlot(title='ADC')
        self.data_line.addLegend()
        self.data_line.setLabels(bottom='Tiempo (s)', left='Voltaje (V)')
        self.data_line1 = self.data_line.plot(self.x1, self.y1, pen=pg.mkPen(color=(255, 0, 0)), name = 'RTU 1')
        self.data_line2 = self.data_line.plot(self.x1, self.y2, pen=pg.mkPen(color=(0, 255, 0)), name = 'RTU 2')
        self.timer = QtCore.QTimer()
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.update_data)
        self.timer.start()

    def update_data(self):
        global mensaje, RTU, evento, hora, botones, switches, leds, adc, hora_tmp, buffer_tmp

        self.x1 = self.x1[1:]  # Remove the first y element.
        self.x1.append(self.x1[-1] + 1)  # Add a new value 1 higher than the last.
        self.y1 = self.y1[1:]  # Remove the first
        self.y1.append(float(adc[1]))  # Add a new random value.
        self.y2 = self.y2[1:]  # Remove the first
        self.y2.append(float(adc[0]))  # Add a new random value.
        self.data_line1.setData(self.x1, self.y1, pen=pg.mkPen(color=(255, 0, 0)))
        self.data_line2.setData(self.x1, self.y2, pen=pg.mkPen(color=(0, 255, 0)))

        self.horizontalSlider.setValue(int(switches[0][0]))
        self.horizontalSlider_2.setValue(int(switches[0][1]))
        self.horizontalSlider_5.setValue(int(switches[0][2]))
        self.lcdNumber.display(float(adc[0]))

        self.horizontalSlider_3.setValue(int(switches[1][0]))
        self.horizontalSlider_4.setValue(int(switches[1][1]))
        self.horizontalSlider_6.setValue(int(switches[1][2]))
        self.lcdNumber_2.display(float(adc[1]))

        for i in buffer_tmp[0]:
            k = re.findall('HORA:(.+)[.]', i)
            if k != hora_tmp[0]:
                hora_tmp[0] = k
                self.verticalLayout_3.addWidget(QtWidgets.QLabel(i))
                buffer_tmp[0] = ['']

        for j in buffer_tmp[1]:
            k = re.findall('HORA:(.+)[.]', j)
            if k != hora_tmp[1]:
                hora_tmp[1] = k
                self.verticalLayout_4.addWidget(QtWidgets.QLabel(j))
                buffer_tmp[1] = ['']


    def presionado1(self):
        global leds_tx
        if int(leds_tx[0][1]):
            leds_tx[0][1] = '0'
        else:
            leds_tx[0][1] = '1'

    def presionado2(self):
        global leds_tx
        if int(leds_tx[0][0]):
            leds_tx[0][0] = '0'
        else:
            leds_tx[0][0] = '1'

    def presionado3(self):
        global leds_tx
        if int(leds_tx[1][0]):
            leds_tx[1][0] = '0'
        else:
            leds_tx[1][0] = '1'

    def presionado4(self):
        global leds_tx
        if int(leds_tx[1][1]):
            leds_tx[1][1] = '0'
        else:
            leds_tx[1][1] = '1'

def PIPES_RX():
    global ventanamain, mensaje, RTU, evento, hora, botones, switches, leds, adc
    print("#RTU | #EVENTO |           TIME STAMP           |   ESTATUS   | VOLTAJE")
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
                    RTU = int(display[0][-1])-1
                    evento[RTU] = display[1]
                    hora[RTU] = str(time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime(float(display[2])))) + '.' + str(float(display[3])//1000)
                    botones[RTU][0] = display[4]
                    botones[RTU][1] = display[5]
                    switches[RTU][0] = display[6]
                    switches[RTU][1] = display[7]
                    switches[RTU][2] = display[8]
                    leds[RTU][0] = display[9]
                    leds[RTU][1] = display[10]
                    adc[RTU] = display[11]
                    buffer_tmp[RTU].append("EVENTO: {} | BOTONES: {};{} | HORA: {}.".format(evento[RTU], botones[RTU][0], botones[RTU][1], hora[RTU]))
                    print("  {}  |    {}    | {} | {};{};{};{};{};{} | {} ".format(RTU+1, evento[RTU], hora[RTU], botones[RTU][0], botones[RTU][1], switches[RTU][0], switches[RTU][1], leds[RTU][0], leds[RTU][1], adc[RTU]))
                    mensaje = ''
            except:
                pass

    return

def PIPES_TX():
    global ventanamain, mensaje, RTU, evento, hora, botones, switches, leds, adc, leds_tx
    with open(FIFO2, "w") as fifo2:
        print("FIFO2 opened")
        while True:
            time.sleep(2)
            fifo2.write('{}.{}.1\n'.format(leds_tx[0][0], leds_tx[0][1]))
            fifo2.flush()
            fifo2.write('{}.{}.2\n'.format(leds_tx[1][0], leds_tx[1][1]))
    return

app = QtWidgets.QApplication([])
stylesheet = Style.StyleSheets("dark_orange")
app.setStyleSheet(stylesheet)
ventanamain=APP()
ventanamain.show()
app.exec_()
