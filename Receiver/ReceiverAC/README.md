# ReceiverACPS #

KBUS Receiever for:

- Atari/Commodore style 9 pin joystick ports  
  - Amiga (1-3 buttons)  
  - C64  
  - Atari 2600 / 7800 / ST etc.  
  - Many other 8/16 bit machines
- PC-Engine (Turbografx-16)
- Sega Master System
- Sega Megadrive (Genesis)

Any other simple switch/multiplex based system can also be supported, with the caveat that de-multiplexing is not as fast as real logic. Sega specify a 2uS delay from changing the multiplex selection to reading the outputs.

The generic XMEGA E5 receiver hardware is used. KBUS specifies 250mA @ 5V. Some systems, such as the Amiga, can't provide this from the joystick port so must get 5V from elsewhere (e.g. the floppy port). Of course not all KBUS devices need 250mA, so you may get away without it.

Lag is sub 1ms ("true zero", or at least 10x better than USB).
