# RFID-Klingel
Arduiono and RFID based Klingel (german for bell).
It will only ring if a know rfid key is read on the door.

Developed and testet with an Arduino Uno Rev3

## Needed Libraries
As today, all needed libraries can be found in the Arduino IDE (Sketch -> Include Library -> Manage Libraries) 
* [MFRC522 by minguelbalboa](https://github.com/miguelbalboa/rfid)
* [u8glib by olikraus](https://github.com/olikraus/u8glib)
* [Adafruit_NeoPixel by Adafruit](https://github.com/adafruit/Adafruit_NeoPixel)
* SPI (arduino default installed lib)
* String (arduino default instaled lib)

## Parts used
The Fritzing files a rough draft and can only used as excample.
* NulSom NeoPixel WS2812B 3x3
* MF-RC522 RF Reader
* 0.96 Zoll I2C IIC OLED LCD Modul MSP420 STIM32 SCR
* Adafruit Audio FX Sound Board - WAV/OGG Trigger with 2MB Flash
* RED LED
* some wires

## TODO
* Change the way the sound board is use. At the moment it uses the GPIO (button) mode. But UART would be a better way to do it, so every guest can have his own uniq bell sound. A Turotial can be found [here](https://learn.adafruit.com/adafruit-audio-fx-sound-board/serial-audio-control) and don't forget to connect UG to ground to enable UART.