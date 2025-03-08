from temp_sensor import TempSensorType, TempSensor
import config
import os

class Max6675Sensor(TempSensor):
    
    def __init__(self):
        super().__init__()
        self.sensor_type = TempSensorType.INTERNAL_HIGH_SENSOR
        base_path = config.CONFIG_MAX6675_SPI_BUS_PATH
        spi_devices = [os.path.join(base_path, d) for d in os.listdir(base_path)]
        for spi_dev in spi_devices:
            props = [d for d in os.listdir(spi_dev)]
            for i in props:
                if "iio:device" in i:
                    self.dev_path = os.path.join(spi_dev, i, "in_temp_raw")
                    return
        
        raise FileNotFoundError("Cannot find max6675")


    def read_value(self):
        fp = open(self.dev_path, "r")
        temp_raw = fp.read().strip()
        fp.close()
        if temp_raw:
            temp_celsius = float(temp_raw) * 1023.75 / 4095
            return temp_celsius
        
        raise IOError("Cannot read value from max6675")
