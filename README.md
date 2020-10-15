# CG SCALE

![alt text](https://github.com/tkristner/CG_SCALE/blob/main/CG_scale_render.png?raw=true)

This is my CG scale based on Sparkfun hardware.

The scale is totally dismountable.
Arms are sliding to allow different fuselage width.
Pads fits in the top holes and allow for multiple wing root size.
LiPo battery allow mobility.

Please bear with me if there are any mistakes.
Do not hesitate to make pull requests.


List of materials:

- 2x Seeed LOAD CELL ( YZC-1B ) 0-3KG (if another loadcells fit any 4 wire analog loadcell is ok) cells are fixed with 6mm metal screws. Please solder the loadcell foiled mass to avoid noise in analog signal.
- 2x Sparkfun QWIIC SCALE - NAU7802
- 1x SparkFun Thing Plus - ESP32 WROOM
- 1x SparkFun Qwiic Mux Breakout - 8 Channel (TCA9548A)
- 1x Button (to switch between Wing peg distances)
- 1x Sparkfun MICRO OLED BREAKOUT QWIIC
- 1x LiPo 1S 3.7V ~750mAh to make it portable (recharged through the SparkFun Thing Plus usb connection)
- 1x Switch placed between board LiPo port and the LiPo battery. This allow to power up the scale when on battery or to charge battery when connected to Sparkfun USB)
