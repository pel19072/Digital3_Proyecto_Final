'''
python -m PyQt5.uic.pyuic -x GUI.ui -o Interfaz.py --> to generate the python
code from the ui created in PyQtDesigner
'''
#************************** GUI Related Imports *******************************#
from Interfaz import *
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import pyqtSlot, Qt, QPoint
from PyQt5.QtGui import QFont, QEnterEvent, QPixmap
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QDialog
from PyQt5 import QtCore
import Style

#************************* Historiador Related Imports ************************#
import os
import sys
import time
import errno
import struct
import re
import threading

#**************************** Variables Globales ******************************#
# Pipes
FIFO = 'CtoPy'
FIFO2 = 'PytoC'
# Historiador
mensaje = '' # Buffer de notificación única
RTU = 0 # Identificador
'''
    Cada característica a continuación es una lista con dos espacios, uno para
    registrar cada RTU de forma independiente
'''
evento = [0,0]
hora = [0.0, 0.0]
botones = [[0,0], [0,0]]
switches = [[0,0,0], [0,0,0]]
leds = [[0,0], [0,0]]
leds_tx = [[0,0], [0,0]]
adc = [0.0, 0.0]

'''
    Variables que se usan como almacenamientos temporales.
        - hora_tmp es usado para comparar horas de los eventos a mostar para
        evitar tener notificaciones repetidas en la Interfaz
        - buffer_tmp es usado para almacenar la información a imprimir de todas
        las notificaciones recibidas cada dos segundos que el RTU manda datos
        para poder agregarlas todas
'''
hora_tmp = ['','']
buffer_tmp = [[''],['']]

'''
    Variable de almacenamiento de la información que ha llegado para imprimir si
    se solicita por el usuario
'''
buffer_terminal = []

# Creación de los pipes
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
        #******************************* Events *******************************#
        self.pushButton.clicked.connect(self.presionado1)
        self.pushButton_2.clicked.connect(self.presionado2)
        self.pushButton_3.clicked.connect(self.presionado3)
        self.pushButton_4.clicked.connect(self.presionado4)
        self.pushButton_5.clicked.connect(self.despliegue)
        #****************************** Threads *******************************#
        rx_hist = threading.Thread(daemon=True,target=PIPES_RX)
        rx_hist.start()
        tx_hist = threading.Thread(daemon=True,target=PIPES_TX)
        tx_hist.start()
        #************************** Real Time Graph ***************************#
        self.x1 = list(range(10))  # Para intervalos de tiempo de 10 unidades
        self.y1 = [3.3 for _ in range(10)]  # 10 data points initialized in 3.3
        self.y2 = [3.3 for _ in range(10)]
        self.data_line =  self.widget.addPlot(title='ADC') # Creación del Plot
        self.data_line.addLegend()
        self.data_line.setLabels(bottom='Tiempo (s)', left='Voltaje (V)')
        # Para poder tener múltiples gráficos en el mismo plano
        self.data_line1 = self.data_line.plot(self.x1, self.y1, pen=pg.mkPen(color=(255, 0, 0)), name = 'RTU 2')
        self.data_line2 = self.data_line.plot(self.x1, self.y2, pen=pg.mkPen(color=(0, 255, 0)), name = 'RTU 1')
        #******************************* Timer ********************************#
        '''
            Este timer está configurado para que cuando se de el timeout se
            actualicen los datos de las gráficas. Esta actualización está hecha
            para que se haga cada 1000ms o 1s
        '''
        self.timer = QtCore.QTimer()
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.update_data)
        self.timer.start()

    #******************************** Handlers ********************************#
    '''
        Nótese que las siguientes funciones corresponden a un handler de cada
        evento, ya sea de el click de algún botón o del timeout del timer. En
        todos los handlers se utilizan las variables globales para que realmente
        afecten a las mismas variables
    '''
    def update_data(self):
        global mensaje, RTU, evento, hora, botones, switches, leds, adc, hora_tmp, buffer_tmp
        self.x1 = self.x1[1:]  # Quita el primer valor
        self.x1.append(self.x1[-1] + 1)  # Agrega un segundo más
        self.y1 = self.y1[1:]  # Quita el primer valor
        self.y1.append(float(adc[1]))  # Agrega el nuevo valor sensado
        self.y2 = self.y2[1:]
        self.y2.append(float(adc[0]))
        # Actualiza las gráficas
        self.data_line1.setData(self.x1, self.y1, pen=pg.mkPen(color=(255, 0, 0)))
        self.data_line2.setData(self.x1, self.y2, pen=pg.mkPen(color=(0, 255, 0)))
        # Actualiza los datos del RTU1
        self.horizontalSlider.setValue(int(switches[0][0])) # Switch 1
        self.horizontalSlider_2.setValue(int(switches[0][1])) # Switch 2
        self.horizontalSlider_5.setValue(int(switches[0][2])) # IoT
        self.lcdNumber.display(float(adc[0])) # ADC
        # Actualiza los datos del RTU2
        self.horizontalSlider_3.setValue(int(switches[1][0]))
        self.horizontalSlider_4.setValue(int(switches[1][1]))
        self.horizontalSlider_6.setValue(int(switches[1][2]))
        self.lcdNumber_2.display(float(adc[1]))
        # Agrega las notificaciones del RTU1
        for i in buffer_tmp[0]:
            # Uso de regular expressions (re) para encontrar la hora
            k = re.findall('HORA:(.+)[.]', i)
            if k != hora_tmp[0]:
                '''
                    Solo agrega la notificación si la hora es distinta al del
                    evento anterior (para evitar repetidos, es decir, es
                    programación defensiva)
                '''
                hora_tmp[0] = k
                self.verticalLayout_3.addWidget(QtWidgets.QLabel(i))
                buffer_tmp[0] = ['']
        # Agrega las notificaciones del RTU2
        for j in buffer_tmp[1]:
            k = re.findall('HORA:(.+)[.]', j)
            if k != hora_tmp[1]:
                hora_tmp[1] = k
                self.verticalLayout_4.addWidget(QtWidgets.QLabel(j))
                buffer_tmp[1] = ['']
    '''
        Los handlers del tipo "presionadon" tienen la misma lógica, solo cambia
        qué LED afecta. El handler presionado1 afecta al LED1 del RTU 1, el
        handler presionado2 afecta al LED2 del RTU 1, el handler presionado3
        afecta al LED1 del RTU 2, el handler presionado4 afecta al LED2 del
        RTU 2
    '''
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

    # Muestra los datos en la terminal al presionar el botón
    def despliegue(self):
        global buffer_terminal
        for i in buffer_terminal:
            print(i)

#********************************** Recepción *********************************#
def PIPES_RX():
    global ventanamain, mensaje, RTU, evento, hora, botones, switches, leds, adc, buffer_tmp, buffer_terminal
    # Da un print para el formato general de las notificaciones del historiador
    print("#RTU | #EVENTO |           TIME STAMP           |   ESTATUS   | VOLTAJE")
    '''
        - Abre el Pipe para leerlo.
        - La lectura debe hacerse en modo binario por el tipo de dato enviado
        desde C
    '''
    with open(FIFO, "rb") as fifo:
        print("FIFO opened")
        while True:
            try:
                # Lee dato binario uno por uno
                data = fifo.read(1)
                if len(data) == 0:
                    print("Writer closed")
                    break
                '''
                    Se usa el módulo struct para convertir el tipo de dato
                    recibido en C a un tipo de dato de Python
                '''
                mensaje += struct.unpack("=s", data)[0].decode("utf-8")
                '''
                    Cuando se encuentra el terminador de línea se considera que
                    el mensaje ya se recibió completo, se separa por comas y se
                    guarda cada valor según el formato en que el RTU está
                    mandando los datos
                '''
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
                    buffer_terminal.append("  {}  |    {}    | {} | {};{};{};{};{};{} | {} ".format(RTU+1, evento[RTU], hora[RTU], botones[RTU][0], botones[RTU][1], switches[RTU][0], switches[RTU][1], leds[RTU][0], leds[RTU][1], adc[RTU]))
                    buffer_tmp[RTU].append("EVENTO: {} | BOTONES: {};{} | HORA: {}.".format(evento[RTU], botones[RTU][0], botones[RTU][1], hora[RTU]))
                    mensaje = ''
            except:
                pass

    return

#************************************ Envío ***********************************#
def PIPES_TX():
    global ventanamain, mensaje, RTU, evento, hora, botones, switches, leds, adc, leds_tx
    '''
        - Abre el Pipe para escribirle.
        - La escritura debe hacerse en modo normal, contrario a la lectura
    '''
    with open(FIFO2, "w") as fifo2:
        print("FIFO2 opened")
        while True:
            time.sleep(2)
            fifo2.write('{}.{}.1\n'.format(leds_tx[0][0], leds_tx[0][1]))
            fifo2.flush() # Muy importante para que se manden bien los datos
            fifo2.write('{}.{}.2\n'.format(leds_tx[1][0], leds_tx[1][1]))
    return

# Inicializa la aplicación y se le da el estilo importado del documento Style.py
app = QtWidgets.QApplication([])
stylesheet = Style.StyleSheets("dark_orange")
app.setStyleSheet(stylesheet)
ventanamain=APP()
ventanamain.show()
app.exec_()
