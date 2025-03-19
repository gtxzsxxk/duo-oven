from temp_sensor import TempSensorType
from ds18b20_read import Ds18b20Sensor
from max6675_read import Max6675Sensor
from heating_gpio import *
from uvc_read import capture_and_encode
from config import CONFIG_EMERGENCY_BRAKE_INTERVAL
from enum import Enum, auto
import time
import threading
import queue

class HeatingStateMachine(Enum):
    STOP_HEATING = auto()
    DO_STRATEGY = auto()

class HeatingThread(threading.Thread):
    def __init__(self):
        super().__init__()
        self.last_request_time = time.time()
        self.task_queue = queue.Queue()
        self.state = HeatingStateMachine.STOP_HEATING


    def run(self):
        while True:
            if not self.task_queue.empty():
                # Do the tasks
                task = self.task_queue.get_nowait()
                if task == "ping":
                    pass
                elif task.startswith("strategy+"):
                    self.state = HeatingStateMachine.DO_STRATEGY
                elif task == "eb":
                    self.state = HeatingStateMachine.STOP_HEATING
                self.last_request_time = time.time()
            else:
                no_request_interval = time.time() - self.last_request_time
                if no_request_interval > CONFIG_EMERGENCY_BRAKE_INTERVAL:
                    self.state = HeatingStateMachine.STOP_HEATING

            if self.state == HeatingStateMachine.STOP_HEATING:
                heating_disable(0)
                heating_disable(1)
            elif self.state == HeatingStateMachine.DO_STRATEGY:
                # execute the strategy python file
                pass
            time.sleep(0.050)
        

temp_sensor_list = []

def oven_temp_sensors_init():
    temp_sensor_list.append(Ds18b20Sensor())
    temp_sensor_list.append(Max6675Sensor())


def oven_init():
    oven_temp_sensors_init()
    heating_gpio_init()

def oven_temp_internal():
    for i in temp_sensor_list:
        if i.sensor_type == TempSensorType.INTERNAL_HIGH_SENSOR:
            return i.read_value()

    raise FileNotFoundError("Cannot find internal temperature")

def oven_temp_external():
    for i in temp_sensor_list:
        if i.sensor_type == TempSensorType.EXTERNAL_LOW_SENSOR:
            return i.read_value()

    raise FileNotFoundError("Cannot find external temperature")

def camera_capture_base64():
    return capture_and_encode()

if __name__ == "__main__":
    oven_init()
    
    while True:
        for i in temp_sensor_list:
            i.print_value()

        heating_enable(1)
        time.sleep(1)
        heating_disable(1)
        time.sleep(1)
