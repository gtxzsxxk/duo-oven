from temp_sensor import TempSensor
from ds18b20_read import Ds18b20Sensor
from max6675_read import Max6675Sensor

temp_sensor_list = []

def oven_temp_sensors_init():
    temp_sensor_list.append(Ds18b20Sensor())
    temp_sensor_list.append(Max6675Sensor())


if __name__ == "__main__":
    oven_temp_sensors_init()
    for i in temp_sensor_list:
        i.print_value()
