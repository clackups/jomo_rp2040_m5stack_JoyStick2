```
arduino-cli core --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json install rp2040:rp2040



arduino-cli compile --fqbn rp2040:rp2040:rpipico --warnings all --upload -p /dev/ttyACM0 && picocom -b 115200 /dev/ttyACM0

```
