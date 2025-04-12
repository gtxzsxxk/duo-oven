from temp_sensor import TempSensorType
from ds18b20_read import Ds18b20Sensor
from max6675_read import Max6675Sensor
from heating_gpio import *
from uvc_read import capture_and_encode
from strategy_manager import sm_strategy_load_code, sm_strategy_register, sm_strategy_exec
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
        self.task_queue = queue.Queue()
        self.state = HeatingStateMachine.STOP_HEATING
        self.code = ""
        self.heating_arg = ""
        self.heating_working_seconds = 0
        self.heating_interval = 0.1


    def run(self):
        while True:
            time_start = time.time()
            if not self.task_queue.empty():
                # Do the tasks
                task = self.task_queue.get_nowait()
                if task.startswith("strategy=="):
                    self.state = HeatingStateMachine.DO_STRATEGY
                    filename = task.split('==')[1]
                    self.heating_arg = task.split('==')[2]
                    self.code = sm_strategy_load_code(filename)
                    self.heating_working_seconds = 0
                    print("new strategy", filename)
                elif task == "eb":
                    self.state = HeatingStateMachine.STOP_HEATING
                    print("Emergency Brake")

            if self.state == HeatingStateMachine.STOP_HEATING:
                heating_disable(0)
                heating_disable(1)
                self.code = ""
            elif self.state == HeatingStateMachine.DO_STRATEGY:
                if self.code != "":
                    sm_strategy_register(self.code)
                    try:
                        ret = sm_strategy_exec(oven_temp_internal_buffered(), oven_temp_external_buffered(), self.heating_working_seconds, self.heating_arg)
                        if ret == 2:
                            # 主动结束逻辑
                            self.state = HeatingStateMachine.STOP_HEATING
                        self.heating_working_seconds += self.heating_interval
                    except:
                        pass

            time_delta = time.time() - time_start
            if time_delta < self.heating_interval:
                time.sleep(self.heating_interval - time_delta)


temp_sensor_list = []
heating_thread = HeatingThread()

def oven_temp_sensors_init():
    temp_sensor_list.append(Ds18b20Sensor())
    temp_sensor_list.append(Max6675Sensor())


def oven_init():
    oven_temp_sensors_init()
    heating_gpio_init()
    heating_thread.start()


def oven_strategy_start(filename, arg):
    heating_thread.task_queue.put("strategy==" + filename + "==" + arg)


def oven_strategy_stop():
    heating_thread.task_queue.put("eb")


def oven_is_heating():
    return heating_thread.state == HeatingStateMachine.DO_STRATEGY


__internal_temp_last_data = 0
__internal_temp_last_time = 0
def oven_temp_internal_buffered():
    global __internal_temp_last_data, __internal_temp_last_time
    if time.time() - __internal_temp_last_time >= 0.2:
        for i in temp_sensor_list:
            if i.sensor_type == TempSensorType.INTERNAL_HIGH_SENSOR:
                __internal_temp_last_data = i.read_value()
                __internal_temp_last_time = time.time()
                return __internal_temp_last_data
    else:
        return __internal_temp_last_data


__external_temp_last_data = 0
__external_temp_last_time = 0
def oven_temp_external_buffered():
    global __external_temp_last_data, __external_temp_last_time
    if time.time() - __external_temp_last_time >= 1.5:
        for i in temp_sensor_list:
            if i.sensor_type == TempSensorType.EXTERNAL_LOW_SENSOR:
                __external_temp_last_data = i.read_value()
                __external_temp_last_time = time.time()
                return __external_temp_last_data
    else:
        return __external_temp_last_data


def camera_capture_base64():
    return capture_and_encode()

def heating_state_str():
    state_str = ""
    for i in heating_state:
        state_str += "1" if i else "0"
    return state_str

if __name__ == "__main__":
    oven_init()
    
    while True:
        for i in temp_sensor_list:
            i.print_value()

        heating_enable(1)
        time.sleep(1)
        heating_disable(1)
        time.sleep(1)
