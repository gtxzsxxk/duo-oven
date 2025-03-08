from enum import Enum
from abc import ABCMeta, abstractmethod

class TempSensorType(Enum):
    UNDEFINED_SENSOR = 0
    INTERNAL_HIGH_SENSOR = 1
    EXTERNAL_LOW_SENSOR = 2

class TempSensor:
    __metaclass__ = ABCMeta
    
    def __init__(self):
        self.sensor_type = TempSensorType.UNDEFINED_SENSOR


    @abstractmethod
    def read_value(self):
        pass


    def print_value(self):
        value = self.read_value()
        if self.sensor_type == TempSensorType.INTERNAL_HIGH_SENSOR:
            print("Internal: %.2f °C" % value)
        elif self.sensor_type == TempSensorType.EXTERNAL_LOW_SENSOR:
            print("External: %.2f °C" % value)
        else:
            raise TypeError("Uninitialized Sensor")
