from ds18b20_read import Ds18b20Sensor
from max6675_read import Max6675Sensor
from heating_gpio import *
import time

temp_sensor_list = []

def oven_temp_sensors_init():
    temp_sensor_list.append(Ds18b20Sensor())
    temp_sensor_list.append(Max6675Sensor())


if __name__ == "__main__":
    oven_temp_sensors_init()
    heating_gpio_init()
    
    while True:
        for i in temp_sensor_list:
            i.print_value()

        heating_enable(1)
        time.sleep(1)
        heating_disable(1)
        time.sleep(1)
