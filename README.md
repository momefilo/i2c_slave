Ein micropython- Port für pico und pico_w mit i2c_slave-Modul\
Das Modul wirt import i2c_slave impoertiert und vor Benutzung durch Aufruf von\
i2c_slave.init(i2c-Bus, i2c-Adresse SDA-Pin, SCL-Pin, i2c-Frequenz, Callback-Funktion) initialisiert werden,\
und mit i2c_slave.deinit(i2c-Bus) vor neu initialisierung deinitialisiert werden\
Die Callbackfunktion muss den i2c-Bus als ersten, und den i2c-Slavehandle als zweiten Parameter uebergeben bekommen (Siehe i2c_slave_demo_adc.py)\
Der Slavehandle hat drei moegliche Werte: "I2C_SLAVE_RECEIVE", "I2C_SLAVE_REQUEST" und "I2C_SLAVE_FINISH" und kann mit if/else abgefragt werden\

Das Modul bietet zwei Lese- und zwei Schreibfunktionen\
i2c_slave.readByte(i2c_bus) liest ein Byte\
i2c_slave.readBlock(i2c_bus Pointer, Laenge) liest len Bytes in den Speicher Pointer und ich habe noch keine python-Implementierung gefunden\
i2c_slave.writeByte(i2c_bus) schreibt ein Byte\
i2c_slave.readBlock(i2c_bus Pointer, Laenge) schreibt len Bytes aus dem Speicher Pointer auf den i2c-Bus und ich habe noch keine python-Implementierung gefunden\

Bekannte Fehler:
Mit einem RPi als Master wird das erste Daten-Byte oft als 0xFF empfangen
Abhilfe: Einmalig mit dem Master zu erst eine leere Adresse oder i2cdetect aufrufen bevor Daten gesendet werden
