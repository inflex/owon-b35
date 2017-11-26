# OWON-B35 (All models)
OWON B35 Mutltimeter data capture and display for Linux

Captures, converts, displays and stores output from Bluetooth (BLE) output of the OWON B35(T+) series multimeters for Linux.

# Requirements
gatttool needs to be installed and operational in linux for owon-cli to work.

# Setup

1) Build owoncli
 make

2) Find the multimeter;
 sudo hcitool lescan

3) Run owoncli with the multimeter address as the parameter
 sudo ./owoncli 98:84:E3:CD:C0:E5

The program will display in text the current meter display and also generate a text file called "owon.txt" which can be read in to programs like OpenBroadcaster so you can have a live on-screen-display of the multimeter.
