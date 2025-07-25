# PixmobLEDWristBandControl
Allow control of Pixmob LED wristbands using a M5StickC Plus or similar ESP32


Product Links:

M5StickC Plus: https://s.click.aliexpress.com/e/_oCuU3T0
SH1106 128x64 OLED: https://s.click.aliexpress.com/e/_oDDDovo

(If building your own)
ESp32C3: https://s.click.aliexpress.com/e/_oBs5e7k
IR LED Emitter: https://s.click.aliexpress.com/e/_oCMauCW

References/Credits:
https://github.com/danielweidman/pixmob-ir-reverse-engineering
https://github.com/IvanR3D/pixmob-ir-reverse-engineering
https://github.com/danielweidman/pixmob-ir-reverse-engineering/blob/main/www/js/effects_definitions.js
https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
https://ivanr3d.com/tools/led-wristband/

There are 2 Options: A) Just get it going B) Use the debug version to read codes to create new ones.

A) Just get it working as is:  If just want to download the Prod version to a M5StickC Plus (or similar ESP32):
1) If you are new to Arduino/PlatformIO, suggest googling setup tutorials - there are plenty out there
2) Use the version (main.cpp) and load up in PlatformIO or Arduino IDE (rename to .ino)
3) You will need the following libraries:  Adafruit NeoPixel, IRremoteESP8266, IRsend, Wire, U8g2Lub
4) Download the code to the M5StickC Plus. Using the "B" button on the side near the screen, you can select the modes. Select a mode, then click the "A" button on top to start the sequence. To have repeat, click and hold "B" for a few seconds to enable repeat.  NOTE! I didn't get as far as putting an interrupt on the "B" button, so to get out of repeat mode, just keep clicking it for a few seconds until it catches and breaks out of the loop.



B) If you want to debug more codes using the web interface (https://ivanr3d.com/tools/led-wristband/)
with this as reference (https://ivanr3d.com/tools/led-wristband/js/effects_definitions.js):
1) If you are new to Arduino/PlatformIO, suggest googling setup tutorials - there are plenty out there
2) For PlatformIO: Rename mainDebugToOLED.cpp.txt (remove .txt), then rename main.cpp to main.cpp.txt (avoids compiling issues).  For Arduino IDE: rename to .ino
3) You will need the following libraries:  Adafruit NeoPixel, IRremoteESP8266, IRsend, Wire, U8g2Lub
4) In this case, a knock-off ESP32C3 was used. If using something different, then you will need to change the Board reference in PlatformIO or the ArduinoIDE
5) Will also need an OLED screen and IR emitter LED. In this case, an SH1106 128x64 OLED was used. If using something different, you will have to adjust the screen driver in U8g2
6) Wire up the OLED connections. Wire up the LED using the provided schematic link (https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending). Be sure to use the right GPIO pin
7) Download the code to the ESP32 and run it.  Keep it connected to the computer as it will use the port to receive serial commands from the web interface.
8) Go to the Web interface (https://ivanr3d.com/tools/led-wristband/), connect to the board, then select from the Main Effect and Tail Code. READ ALL THE NOTES on this page as some of the combinations do not work!  https://github.com/danielweidman/pixmob-ir-reverse-engineering/blob/main/www/js/effects_definitions.js
9) Click Send and verify the wristband lights up. If it does, you can read the string of numbers pushed to the OLED screen.  You can take those and create new effects in the "//LED Sequences" section in code.
Make sure to create related function calls and declarations. The easiest way to do this is to search for 
"yellow3Fade6" and see where it is referenced to re-create new entries and selections
10) Of course you will have to modify the inferface but again, can use what is provided to tweak as needed.
