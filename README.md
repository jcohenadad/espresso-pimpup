# Espresso pimp up ☕

DIY project to pimp up my espresso machine to display various measures

## Material

- [Ultrasonic sensor HC-SR04](https://www.amazon.ca/dp/B07RGB4W8V?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
- Temperature
  - [K Type Thermocouple Temperature Sensor](https://www.amazon.ca/MECCANIXITY-Thermocouple-Temperature-Sensor-Stainless/dp/B0C998V2DP)
  - [Thermocouple Amplifier MAX31855](https://www.amazon.ca/dp/B00SK8NDAI)
- Water level
  - [FDC1004DGSR](https://www.digikey.ca/en/products/detail/texas-instruments/FDC1004DGSR/5250523) | 4-channel 16-bit capacitance-to-digital converter
  - [LCQT-MSOP10](https://www.digikey.ca/en/products/detail/aries-electronics/LCQT-MSOP10/4754589) | To create the breakout board for the FDC1004 chip
  - [WR-PHD 2.54 MM SOCKET HEADER 5 P](https://www.digikey.ca/en/products/detail/w%C3%BCrth-elektronik/61300511821/16608594) x 2 | To plug the breakout board on the main PCB
  - [C0603C104M4RACTU](https://www.digikey.ca/en/products/detail/kemet/C0603C104M4RACTU/411098) | Decoupling Capacitors
  - [RC0603FR-074K7L](https://www.digikey.ca/en/products/detail/yageo/rc0603fr-074k7l/727212) | Pull-up Resistors for I2C

## References

This project is inspired by the following sources:
- https://www.youtube.com/watch?v=4V0a9dfkx9Q
- https://github.com/PostModernDesign/RancilioBrain/blob/master/RancilioBrain.md
- https://github.com/urbantrout/bleeding-edge-ranciliopid/tree/master?tab=readme-ov-file
- https://clevercoffee.de/rancilio-silvia-demontage/
- https://clevercoffee.de/gaggia-classic-9480/
- https://clevercoffee.de/rancilio-silvia-e-konstantin/
- https://clevercoffee.de/rancilio-silvia-e-sebi/
- https://www.youtube.com/watch?v=AzQRCUg6E9g
- https://optimalprimate.github.io/projects/2020/11/14/coffee.html
- https://github.com/mwood77/koffie
  - https://www.seattlecoffeegear.com/cdn/shop/files/silvia-pid-installation-guide.pdf?v=8847236618586878957
- https://www.reddit.com/r/ranciliosilvia/search/?q=pid&cId=b4224602-a8c1-4eaf-a334-e28db799d089&iId=4bad28b5-60e2-4aab-9f05-2a9fabc166d9
- https://github.com/brycesub/silvia-pi?tab=readme-ov-file

