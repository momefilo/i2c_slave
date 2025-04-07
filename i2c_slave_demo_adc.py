# momefilo i2c_slave_demo_adc.py
# Das ist ein Demoprogramm für das i2c_slave-Modul  (getestet auf pico_w)
# Verbinde die pico-i2c-pins mit einem RPi und führe volgende Befehle aus
# Mit i2cget -y 1 0x47 (0x00 bis 0x04) i 2 wird zuerst 
# das Highbyte und dann das Lowbyte der ADC(0) bis ADC(4) empfangen
# Mit i2cset -y 1 0x47 0x05 0x01 wird die Onboard-led ein -und
# mit i2cset -y 1 0x47 0x05 0x00 ausgeschaltet

from machine import Pin, ADC
import i2c_slave
import time

led = Pin("LED", Pin.OUT)# an pico ohne w anzupassen
data = 0# Aktueller ADC-Wert
receive = -1# led-Schaltung benoetigte Variable zur Trennung von Adress- und Datenbyte

def set_led(val):
  if val == 0:
    led.off()
  else:
    led.on()

def get_adc(val):
  global data
  data = ADC(val).read_u16()

# Diese Funktion wird zur Initialisierung benötigt
# und wird ausgeführt wenn der Master eine read/write-Nachricht an unsere Adresse sendet
# Als handler sind "I2C_SLAVE_RECEIVE", "I2C_SLAVE_REQUEST" und "I2C_SLAVE_FINISH" moeglich
def callback(i2c, handler):
  if handler == "I2C_SLAVE_RECEIVE":
    global receive
    global data
    rec = i2c_slave.readByte(i2c)# das erste Byte nach dem Adressbyte einlesen
    if (rec == 5 and receive == -1):# Das nächste Byte schaltet die led
      receive = rec
    elif receive == 5:# Das Byte welches die led schaltet
      set_led(rec)
      receive = -1
    else:# Die ADC's werden gelesen
      get_adc(rec)
      
  if handler == "I2C_SLAVE_REQUEST":
    i2c_slave.writeByte(i2c, ((data & 0xFF00) >> 8))
    i2c_slave.writeByte(i2c, (data & 0x00FF))
    
  if handler == "I2C_SLAVE_FINISH":
    pass

i2c_slave.init(0, 4, 5, 400*1000, 0x47, callback)
#while True:
#  time.sleep(1)
  
