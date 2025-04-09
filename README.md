## Ein micropython- Port für pico und pico_w mit i2c_slave-Modul
Das Modul wird mit 
```
import i2c_slave
```
importiert und vor Benutzung durch Aufruf von
```
i2c_slave.init(i2c-Bus, SDA-Pin, SCL-Pin, i2c-Frequenz, i2c-Adresse, Callback-Funktion)
```
initialisiert, und mit
```
i2c_slave.deinit(i2c-Bus)
```
vor neu initialisierung deinitialisiert\
Die Callbackfunktion muss den i2c-Bus als ersten, und den i2c-Slavehandle als zweiten Parameter aufnehmen koennen. Sie wird mittels eines IRQ's durch den i2c-Master per read/write aufgerufen. Z.B.
```
def callback (i2c_bus, handle):
    print(i2c_bus, handle)

i2c_slave.init(0, 4, 5, 400*1000, 0x47, callback)
```
Der Slavehandle hat drei moegliche Werte:\
"I2C_SLAVE_RECEIVE"\
"I2C_SLAVE_REQUEST"\
"I2C_SLAVE_FINISH"\
Siehe [i2c_slave_demo_adc.py](i2c_slave_demo_adc.py)

Das Modul bietet zwei Lese- und zwei Schreibfunktionen
```
i2c_slave.readByte(i2c_bus)
```
liest ein Byte als Rueckgabewert
```
i2c_slave.readBlock(i2c_bus, Laenge) 
```
liest Laenge Bytes in eine list als Rueckgabewert
```
i2c_slave.writeByte(i2c_bus, data)
```
schreibt ein Byte
```
liste = [1,2,3,4]
i2c_slave.writeBlock(i2c_bus, liste, 4)
```
schreibt n Bytes aus einer list auf den i2c-Bus

# Bekannte Fehler:
Mit einem RPi als Master wird das erste Daten-Byte oft als 0xFF interpretiert\
Abhilfe:\
Einmalig mit dem Master zu erst eine leere Adresse oder i2cdetect aufrufen

Download MicroPython v1.25.0-preview-firmware for [rp2/pico](pico_i2c_slave.uf2)\
Download MicroPython v1.25.0-preview-firmware for [rp2/pico_w](pico_w_i2c_slave.uf2)
