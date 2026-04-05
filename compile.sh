arduino-cli compile --fqbn arduino:avr:nano ArduinoEspressoSensor/
arduino-cli upload ArduinoEspressoSensor -b arduino:avr:nano -p /dev/cu.usbserial-BG0046HD
arduino-cli monitor -p /dev/cu.usbserial-BG0046HD --config 9600

