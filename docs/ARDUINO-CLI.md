# Setting up dev-environment with arduino-cli

1. Install [arduino-cli](https://arduino.github.io/arduino-cli/0.35/installation/)
2. Init configruation with `arduino-cli config init`
3. Add Raspberry Pi Pico board url to config
   `arduino-cli config add board_manager.additional_urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
4. Update core index `arduino-cli core update-index`
5. Install board core `arduino-cli core install rp2040:rp2040`
6. Optional: command line completion (example for bash:
   `arduino-cli completion bash > arduino-cli.sh && sudo mv arduino-cli.sh /etc/bash_completion.d/`)

You can now create a sketch with `arduino-cli sketch new ./src/MyNewSketch`

## Compiling and uploading to board

1. Compile `arduino-cli compile ./src/MyNewSketch/ --fqbn rp2040:rp2040:rpipico`
2. Upload `arduino-cli upload -p /dev/ttyACM0 --fqbn rp2040:rp2040:rpipico ./src/MyNewSketch` (To
   upload straight away just add -u flag to compile)

Remember to change the port to the one the Pico is connected to. To check the board run:

```bash
$ arduino-cli board list

Port         Protocol Type              Board Name FQBN Core
/dev/ttyUSB0 serial   Serial Port (USB) Unknown                 # <-- my ESP32 board

```

## Installing libraries

Search with `arduino-cli lib search debouncer`

Install with `arduino-cli lib install FTDebouncer`
