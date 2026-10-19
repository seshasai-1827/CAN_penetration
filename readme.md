This is a project looking to provide protection to CAN comms against external attackers and unexpected traffic of frames.

Run to setup socketCAN and a virtual CAN link : 

sudo apt update
sudo apt install can-utils build-essential

sudo modprobe vcan
sudo modprobe can-isotp

sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

To view link setup : 
ip -details link show vcan0

To validate CAN connection :
lsmod | grep -E 'can_isotp|can-isotp'

To build the test project: 
cmake -S . -B build
cmake --build build -j