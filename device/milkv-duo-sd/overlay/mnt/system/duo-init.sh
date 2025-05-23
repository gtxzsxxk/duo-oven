#!/bin/sh

# Set Pin-29(GP22) to GPIO
devmem 0x0502707c 32 0x111
devmem 0x03001068 32 0x3

# Set Pin-19(GP14) to GPIO
duo-pinmux -w GP14/GP14 > /dev/null

# Insmod PWM Module
insmod /mnt/system/ko/cv180x_pwm.ko

# Configure eth0 and route information
ip addr add 192.168.9.100/24 dev eth0
ip link set eth0 up
ip route add default via 192.168.9.1 dev eth0

sh /mnt/system/start_server.sh &
