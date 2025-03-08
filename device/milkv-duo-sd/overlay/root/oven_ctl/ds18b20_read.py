from temp_sensor import TempSensorType, TempSensor
import config
import os

class Ds18b20Sensor(TempSensor):
    
    def __init__(self):
        super().__init__()
        self.sensor_type = TempSensorType.EXTERNAL_LOW_SENSOR
        base_path = config.CONFIG_DS18B20_BUS_PATH
        devices = [d for d in os.listdir(base_path) if not d.startswith("w1_bus_master")]
        if len(devices) == 0:
            raise FileNotFoundError("Cannot find ds18b20")
        self.dev_path = os.path.join(base_path, devices[0], "temperature")


    def read_value(self):
        fp = open(self.dev_path, "r")
        temp_raw = fp.read().strip()
        fp.close()
        if temp_raw:
            temp_celsius = float(temp_raw) / 1000
            return temp_celsius
        
        raise IOError("Cannot read value from ds18b20")
