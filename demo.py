# momefilo i2c_slave
from machine import Pin
import i2c_slave
import time

def callback(i2c, handler):
  if handler == "I2C_SLAVE_RECEIVE":
    print("py_read", i2c_slave.read(i2c))
  if handler == "I2C_SLAVE_REQUEST":
    print("py_send")
    i2c_slave.write(i2c, 0xAA)
    i2c_slave.write(i2c, 0xBB)
    i2c_slave.write(i2c, 0xCC)
  if handler == "I2C_SLAVE_FINISH":
    print("py_finish")

i2c_slave.init(0, 4, 5, 400*1000, 0x47, callback)
while True:
  time.sleep(1)
  
