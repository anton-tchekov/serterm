# serterm
Very simple serial terminal for Linux

Build:

`make`

Run:

`./serterm port baudrate`

For example:

`./serterm /dev/ttyACM0 9600`

When ran with no parameters, it prints a list of all devices
that match `/dev/ttyUSB*` and `/dev/ttyACM*`.
