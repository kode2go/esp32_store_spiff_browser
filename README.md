# esp32_store_spiff_browser

Follow on from:
https://github.com/kode2go/temperature_iot

Note: uploading new file will delete existing 

IP Address: http://esp32fs.local

# Setup with VS Code + Platform IO ( a lot quicker upload time )

https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/

platform.ini file
```
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
lib_deps =
  # RECOMMENDED
  # Accept new functionality in a backwards compatible manner and patches
  blynkkk/Blynk @ ^1.3.2
```

Source code is similar to app.ino

just has the following at very top:

```
#include <Arduino.h>
```
