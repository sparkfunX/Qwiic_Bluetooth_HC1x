"C:\Program Files (x86)\Arduino\hardware\tools\avr/bin/avrdude" -C%LocalAppData%\Arduino15\packages\ATTinyCore\hardware\avr\1.1.5/avrdude.conf -v -pattiny85 -cusbtiny -e -Uefuse:w:0xFF:m -Uhfuse:w:0b11010111:m -Ulfuse:w:0xF1:m 

"C:\Program Files (x86)\Arduino\hardware\tools\avr/bin/avrdude" -C%LocalAppData%\Arduino15\packages\ATTinyCore\hardware\avr\1.1.5/avrdude.conf -v -pattiny85 -cusbtiny -Uflash:w:%LocalAppData%\Arduino15\packages\ATTinyCore\hardware\avr\1.1.5/bootloaders/empty/empty_all.hex:i 

"C:\Program Files (x86)\Arduino\hardware\tools\avr/bin/avrdude" -C"C:\Program Files (x86)\Arduino\hardware\tools\avr/etc/avrdude.conf" -v -pattiny85 -cusbtiny -Uflash:w:Qwiic_BT_Tiny.ino-v1.0.0.hex:i