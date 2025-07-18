#!/usr/bin/sh

# Add the vcan kernel module and create/setup the virtual CAN interface

sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
