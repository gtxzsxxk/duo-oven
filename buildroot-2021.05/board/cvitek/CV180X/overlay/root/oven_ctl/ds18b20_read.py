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
        device = os.path.join(base_path, devices[0], "temperature")
        self.dev_fp = open(device, "r")


    def read_value(self):
        temp_raw = self.dev_fp.read().strip()
        if temp_raw:
            temp_celsius = float(temp_raw) / 1000
            return temp_celsius
        
        raise IOError("Cannot read value from ds18b20")


    def __del__(self):
        close(self.dev_fp)
