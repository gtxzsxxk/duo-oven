from pinpong.board import Board,Pin
from config import CONFIG_HEATING_GPIO

heating_gpios = []
heating_state = []

def heating_gpio_init():
    Board("MILKV-DUO").begin()
    for gpio in CONFIG_HEATING_GPIO:
        heating_gpios.append(Pin(gpio, Pin.OUT))
        heating_state.append(False)
        heating_gpios[len(heating_gpios) - 1].value(0)


def heating_enable(idx):
    heating_gpios[idx].value(1)
    heating_state[idx] = True


def heating_disable(idx):
    heating_gpios[idx].value(0)
    heating_state[idx] = False
