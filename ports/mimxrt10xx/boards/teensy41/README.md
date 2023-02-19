# Teensy 4.1 support for UF2

## Will this work?
I think what's in loader-loader should be roughtly what does it. I
didn't bring a teensy with my on vacation, so can't try it. 

I checked the .elf file and the highest used itcm address in the
genearted file is well below 0x1000, so I believe the whole ITCM is
available to just copy the tinyuf2 over at the 0x1000 address.

Anything wrong with this approach?

run the file `loader-loader/convert-to-header.py` to convert the
`tinyuf2.bin` file to a `.h` file, copy and jump.

Should work, right?



## older hacky stuff

To get tinyuf2 to run on the Teensy, we need to load it properly.  The known working method is
get the teensy into the ROM bootloader mode, then load tinyuf2.

## Requirements
You need platformio to build and upload the sketch.  You also need the [nxp_blhost_sdphost](https://github.com/apexrtos/nxp_blhost_sdphost) tools.


Build nxp_blhost_sdphost:
```bash
git submodule update --init ../../../../lib/nxp_blhost_sdphost
pushd ../../../../lib/nxp_blhost_sdphost && make && popd
```

Make sure you have platformio installed
```bash
pip install platformio
```

## Now, you should be able to enter the ROM bootloader
NOTE:  you MUST have pin 25 of the teensy pulled high.  This is
GPIO_AD_B0_13, which is the UART bootloader pin.  If this is low, then
the ROM bootloader thinks you want to load from UART.  I mean... maybe
you do, but I don't for now.

This is easy to accomplish with tweezers betwee pin 25 and the nearly
adjacent 3.3V 2 pins away.


### The way that works for now:
upload a bogus hex file
```bash
# This puts the board into ROM bootloader mode.
teensy_loader_cli --mcu TEENSY41 tinyuf2-teensy41.hex
```


### This way doesn't work yet... 
This enters the bootloader, but I think some stuff is left initialized wrong.
Perhaps the memory map.

```bash
pushd enter-bootloader
pio run --target upload  # This loads a sketch that jumps to the ROM bootloader
popd
```


## And finally, build and upload tinyuf2
```bash
cd _build/teensy41
sdphost -u 0x1fc9,0x0135 write-file 0x1000 tinyuf2-teensy41.bin
sdphost -u 0x1fc9,0x0135 jump-address 0x2000
```
