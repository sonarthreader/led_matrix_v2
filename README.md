# LED Matrix V2
Display PNG files on a LED matrix or watch some audio spectrum analysis. 

Part list:

- ESP32 DEVKIT
- LED Panel SK6812 WS2812B 16x16
- Rotary Encoder KY-040 (2x)
- Digital microphone INMP441
- Mean Well 5V LRS-50-5 Power Supply 50W 10A

This improved version has two knobs to switch between slide show mode and audio analysis (party mode :) ) and to change brightness and pattern.

![Layout](/LED_Matrix_V2_Steckplatine.png)

Valid PNG files are 16px * 16px as created on https://www.pixilart.com/. There is NO scaling for other heights/widths.

Example code used from https://github.com/bitbank2/PNGdec, https://github.com/s-marley/ESP32_FFT_VU and https://github.com/smford/esp32-asyncwebserver-fileupload-example (and probably more)
