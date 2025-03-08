from temp_sensor import TempSensorType, TempSensor
import config
import os

class Max6675Sensor(TempSensor):
    
    def __init__(self):
        super().__init__()
        self.sensor_type = TempSensorType.INTERNAL_HIGH_SENSOR
        base_path = config.CONFIG_MAX6675_SPI_BUS_PATH
        spi_devices = [d for d in os.listdir(base_path)]
        for spi_dev in spi_devices:
            props = [d for d in os.listdir(spi_dev)]
            for i in props:
                if "iio:device" in i:
                    self.iio_device = os.path.join(base_path, "spi_dev", i)
                    self.dev_fp = open(self.iio_device, "r")
                    return
        
        raise FileNotFoundError("Cannot find max6675")


    def read_value(self):
        temp_raw = self.dev_fp.read().strip()
        if temp_raw:
            temp_celsius = float(temp_raw) * 1023.75 / 4095
            return temp_celsius
        
        raise IOError("Cannot read value from max6675")


    def __del__(self):
        close(self.dev_fp)
