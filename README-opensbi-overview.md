# Contents of this Branch

This branch captures xv6 changes made to support the port of xv6 to several boards running opensbi (SBI), which is what each board natively runs to support booting Linux.

With proper option selection, the code supports Sifive Unmatched B (unmatched), StarFive VisionFive 2 (vf2) and, to a more limited extent, Banana Pi F3 (bpif3). See platform specific notes in their individual README-opensbi-boardXX.md for further details.

The official MIT xv6 release supports QEMU RISCV machine mode (only), so changes are needed to allow it to coexist with SBI and then to add any board specific options. 

It may be possible to run the official release without SBI on these boards, but that would require running in place of the highly board specic Uboot-SPL, which contains all the low level (e.g. DDR ram) initialization code. 

What we are doing instead is interrupting Linux boot by pressing a key while it is in uboot and using uboot features to load a SBI compatible binary image instead of Linux. At that time, RISCV is in supervisor mode and can no longer directly access machine mode. Machine mode requests require use of the SBI API.

Some simplifications are made in the current release:

- a r/w ramdisk substitutes for storage, preloaded with the same applications distributed in the official release. A
small utility is provided to repopulate this image into the #included ramdisk header file.
- no PLIC interrupts are enabled
- the only device interrupting xv6 is the timer (and, of course, synchronous interrupts/traps that are part of xv6)
- as a consequence of the above, xv6 continues to use the SBI console beyond booting. 

Given that there are no device interrupts, a trick is employed to get SBI console input. The timer interrupt is used 
to poll for keyboard input, either via SBI API call or directly to the underlying hardware (e.g. sifive uart or 
8250/16550 as used on vf2 and bpif3). The timer interrupt is calibrated for approx 10x per second on the various boards as
was done in QEMU, so this provides sufficient polling rate for console characters.

# Quick Start

For quick start, Git release v0.3.1 includes a zip file containing board specific kernel.bin files that can be run from uboot (each in their own subdirectory).

This is how to run them:

For these instructions, copy the board specific binary to the board's SD card root directory, on its fat partion. Other
partitions can be used but that changes the uboot commands necessary to find them. When in doubt, use uboot to
perform a directory listing of your device (e.g. fatls command or ext4ls as appropriate in order to provide
the right device name, e.g. mmc, partition, e.g. 0:3 and path).

Utilize the board specific debug UART to boot and control XV6. It is assumed that the user has previously gotten
the board to boot Linux using the vendor provided SD card images.

## Unmatched

Unmatched connects to the host PC (running Linux) via USB cable which creates 2 serial interfaces. On my setup, there are no other USB serial devices, so it will create /dev/ttyUSB0 and /dev/ttyUSB1. The debug Uart is on the 2nd
serial port device, e.g. /dev/ttyUSB1.

If in doubt, examine the response to 'dmesg' after plugging in the unmatched board, even with its power off. For example:

```
[ 9519.422949] ftdi_sio 1-3:1.0: FTDI USB Serial Device converter detected
[ 9519.423039] usb 1-3: Detected FT2232H
[ 9519.423316] usb 1-3: FTDI USB Serial Device converter now attached to ttyUSB0
[ 9519.423778] ftdi_sio 1-3:1.1: FTDI USB Serial Device converter detected
[ 9519.423883] usb 1-3: Detected FT2232H
[ 9519.424135] usb 1-3: FTDI USB Serial Device converter now attached to ttyUSB1

```

Notice that 2 USB devices were created, we want the 2nd one for the debug UART.

Run an app like putty and select /dev/ttyUSB1 (or where it appears on your pc) for serial and set the bitrate to 115200. Connect the serial port
prior to powering the board so you can catch the startup sequence. The power up and watch putty.

Interrupt linux boot by quickly pressing a key while the code is still in uboot. This may require a couple of tries
to get the timing right but you will be presented with a uboot prompt if successful.

The code is loaded to 0x80400000, but the path to the files differ owing to the device:partition name.

On unmatched, the SD fat partition is on device 0:3 

At uboot command prompt:
```
=> fatload mmc 0:3 0x80400000 kernel.bin
```

then issue the go command:
```
=> go 0x80400000
```

xv6 should boot to its command prompt where you can do 'ls' and run the precompiled executables. The
code is runing with SMP enabled for all cores.

If the sequence is successful, the startup should be similar to this:

```
Hit any key to stop autoboot:  0
=> fatload mmc 0:3 0x80400000 kernel.bin
2078760 bytes read in 1836 ms (1.1 MiB/s)
=> go 0x80400000
## Starting application at 0x80400000 ...

xv6 kernel is booting

hart 1 starting
hart 4 starting
hart 3 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2488
cat            2 3 35048
echo           2 4 33928
forktest       2 5 16144
grep           2 6 38480
init           2 7 34392
kill           2 8 33864
ln             2 9 33672
ls             2 10 36992
mkdir          2 11 33920
rm             2 12 33904
sh             2 13 56472
stressfs       2 14 34784
usertests      2 15 179136
usertests2     2 16 179136
grind          2 17 49704
wc             2 18 35984
zombie         2 19 33264
console        3 20 0
$
```

## VF2

VF2 debug UART access requires the addition of an inexpensive ftdi USB interface (e.g. Sparkfun ftdi basic) and connecting the TX, RX and gnd signals to the correct pins on the 40 pin header. Note that
the ftdi "TX" pin connects to the board "RX" pin 10, the ftdi "RX" pin connects to the board "TX" pin 8 and gnd
attaches to gnd, pin 6. This crossover of TX & RX is because each device is labeling the pins as respect to itself. 

Please be sure to confirm the pinout with the vendor documentation before making any connections since improper
interconnection can damage the board, interface or (unlikely) the host PC.

The board advertises that its USB connector has a data capability but I'm not sure how to use that given that the USB connector must provide power to the board and PC USB ports dont have enough power.

If using the Sparkfun ftdi basic adapter, when it is plugged into the host PC a single serial port is created.
If this is the only USB serial port on your host machine, it should appear at /dev/ttyUSB0.

If in doubt, examine the host response to 'dmesg' after plugging in the ftdi adapter board, even with its power off. For example:

```
[10497.254844] usb 1-3: Product: FT232R USB UART
[10497.254849] usb 1-3: Manufacturer: FTDI
[10497.254854] usb 1-3: SerialNumber: A104OK20
[10497.258505] ftdi_sio 1-3:1.0: FTDI USB Serial Device converter detected
[10497.258602] usb 1-3: Detected FT232R
[10497.259470] usb 1-3: FTDI USB Serial Device converter now attached to ttyUSB0
```

Run an app like putty and select /dev/ttyUSB0 (or where it appears on your pc) for serial and set the bitrate to 115200. Connect the serial port prior to powering the board so you can catch the startup sequence. The power up and watch putty.

Interrupt linux boot by quickly pressing a key while the code is still in uboot.

The code is loaded to 0x80400000, but the path to the files differ owing to the device:partition name.

On vf2, the SD fat partition is on device 1:3

At uboot command prompt:
```
=> fatload mmc 1:3 0x80400000 kernel.bin
```
then issue the go command:
```
=> go 0x80400000
```

xv6 should boot to its command prompt where you can do 'ls' and run the precompiled executables. The
code is runing with SMP enabled for all cores.

If the sequence is successful, the startup should be similar to this:

```
Hit any key to stop autoboot:  0
StarFive # fatload mmc 1:3 0x80400000 kernel.bin
2078760 bytes read in 96 ms (20.6 MiB/s)
StarFive # go 0x80400000
## Starting application at 0x80400000 ...

xv6 kernel is booting

hart 2 starting
hart 4 starting
hart 3 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2488
cat            2 3 35048
echo           2 4 33928
forktest       2 5 16144
grep           2 6 38480
init           2 7 34392
kill           2 8 33864
ln             2 9 33672
ls             2 10 36992
mkdir          2 11 33920
rm             2 12 33904
sh             2 13 56472
stressfs       2 14 34784
usertests      2 15 179136
usertests2     2 16 179136
grind          2 17 49704
wc             2 18 35984
zombie         2 19 33264
console        3 20 0
$
```

## BPIF3

BPIF3 debug UART access requires the addition of an inexpensive ftdi USB interface (e.g. Sparkfun ftdi basic) and connecting the TX, RX and gnd signals to the TX, RX and gnd labeled pins alongside the 26 pin header. Note that
the ftdi "TX" pin connects to the board "RX" pin, the ftdi "RX" pin connects to the board "TX" pin and gnd
attaches to gnd. THis is because each device is labeling the pins as respect to itself. 

Please be sure to confirm the pinout with the vendor documentation before making any connections since improper
interconnection can damage the board, interface or (unlikely) the host PC.

If using the Sparkfun ftdi basic adapter, when it is plugged into the host PC a single serial port is created.
If this is the only USB serial port on your host machine, it should appear at /dev/ttyUSB0.

If in doubt, examine the host response to 'dmesg' after plugging in the ftdi adapter board, even with its power off. For example:

```
[10497.254844] usb 1-3: Product: FT232R USB UART
[10497.254849] usb 1-3: Manufacturer: FTDI
[10497.254854] usb 1-3: SerialNumber: A104OK20
[10497.258505] ftdi_sio 1-3:1.0: FTDI USB Serial Device converter detected
[10497.258602] usb 1-3: Detected FT232R
[10497.259470] usb 1-3: FTDI USB Serial Device converter now attached to ttyUSB0
```

Run an app like putty and select /dev/ttyUSB0 (or where it appears on your pc) for serial and set the bitrate to 115200. Connect the serial port prior to powering the board so you can catch the startup sequence. The power up and watch putty.

Interrupt linux boot by quickly pressing the space key while the code is still in uboot (only space key!).

The code is loaded to 0x10000000, and the path to the files differ owing to the device:partition name.

On bpif3, the SD fat partition is on device 0:1

At uboot command prompt:
```
=> fatload mmc 0:1 0x10000000 kernel.bin
```
then issue the go command:
```
=> go 0x10000000
```

xv6 should boot to its command prompt where you can do 'ls' and run the precompiled executables. The
code is runing with SMP enabled for all cores, but refer to the board specific README for noted anomalies.

If the sequence is successful, the startup should be similar to this:

```
=> fatload mmc 0:1 0x10000000 kernel.bin
2078760 bytes read in 142 ms (14 MiB/s)
=> go 0x10000000
## Starting application at 0x10000000 ...

xv6 kernel is booting

hart 2 starting
hart 1 starting
hart 3 starting
hart 7 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2488
cat            2 3 35048
echo           2 4 33928
forktest       2 5 16144
grep           2 6 38480
init           2 7 34392
kill           2 8 33864
ln             2 9 33672
ls             2 10 36992
mkdir          2 11 33920
rm             2 12 33904
sh             2 13 56472
stressfs       2 14 34784
usertests      2 15 179136
usertests2     2 16 179136
grind          2 17 49704
wc             2 18 35984
zombie         2 19 33264
console        3 20 0
$
```

Note that only 5 of 8 cores are running on this board (four "starting" and hart 0 starting them). This is the subject of some discussion in the README-opensbi-BananaPi-F3.md file.

# Code compilation

SBI Code modifications (from forked distribution xv6) are activated a selection at the top of Makefile:
 
```
runtime_config := sbi  #this enables the sbi specific changes required by all boards
```
Then choose one of the following board specific configurations (shown here is the one for Banana Pi F3, or bpif3):

```
#board_config := unmatched
#board_config := vf2
board_config := bpif3
```

Unfortunately, not all vendor implementations support SBI console polling (vf2), so for that board, the following define in
param.h must be used to select direct UART polling:

```
#define POLL_UART0_DIRECT
```

The location and nature of UART Rx polling is different across the boards, so the configuration changes
parameters necessary to perform direct uart polling (still within the timer interrupt/trap) instead of SBI API.

The POLL_UART0_DIRECT option works on all 3 boards in the 0.3.1 release. So its ok to use this option on unmatched and bpif3 too. Refer to the param.h file for a table of valid options depending on the board.

With options chosen, compile the code using riscv64-linux-gnu compiler tools that are available in your path. On debian, this command both installs the required tools and inserts it into your PATH:

```
$ sudo apt install gcc-riscv64-linux-gnu
```
(my version of Debian installed v12.2.0)

To compile the code, go into the root directory and perform make:

```
$ make
```

Be sure to have 'make' installed on your host to be able to run it.

make generates an elf and binary version of the kernel in /kernel subdirectory.

The elf version can be used for jtag debug, and the kernel.bin file can be used to run from uboot as described above.

```
$ make clean
```

removes all the compilation artifacts but should not technically be required when changing board configurations. It
does not remove ram_disk.h (see User programs section) because that image is somewhat independent from the
kernel and board specific configurations. However, if some param.h limits are changed this file should really
be regenerated. I have been able to use the same ram_disk.h for all board specific binaries.

# User programs

The file ```kernel/ram_disk.h``` contains a prebuilt image of all the user applications. You can generate this file
by first issuing the following make command from the root directory:

```
$ make userprogs
```

Then 

```
$ cd mk-ramdisk
$ cp ../fs.img .
$ ./fsimg2c
$ cp ram_disk.h ../kernel
$ cd ..
$ make
```

This should really be integrated into the Makefile but for now requires the above steps to recreate the ram_disk.h file and then compile xv6 to include it. Of course, the fsimg2c.c program really should take in parameters..

# Jtag

Coming from a professional embedded firmware development background, I prefer the use of Jtag in debugging and exploration. This mode allows access to all processor registers and hardware (including machine mode) so it is educational and useful for low level debug. All boards support Jtag debugging to varying degrees and in the board specific notes I will describe the hookup and varying experiences.

Note that the apt install from Debian does not include the riscv64 version of gdb, so it is necessary to
get that elsewhere (refer to Jtag info README).

I am using ```riscv64-unknown-linux-gnu-gdb``` for Jtag debug sessions. Refer to the README-Jtag-common README for debug tips and the board specific README for additional info.

# Ideas for Next Steps

Modify the code to eliminate the simplifications:

- Enable device interrupts (PLIC) to support board specific peripherals directly. A first step might be to stop polling for UART input.
- Enable SD card support instead of the ramdisk for storage.  

However, its not clear how much more usable the SD card support makes xv6 since it does not support self compilation of applications (no compiler port) so an external PC is needed to do that anyway and to copy it to an SD containing it and xv6 binaries. The boards all have quite a bit of DDR RAM that can store very large applications for further OS exploration..

Other ideas:
- Try to resolve board specific errata (refer to their README files)
- Add some more interesting user applications (the current ones are geared to OS study)
- Add user mode gpio support?
- Add framebuffer support for video console to avoid having to hook up debug uart?
- Add support for other interesting peripherals? It would be great to boot from nvme but that may be very far off given how much code is needed to support a disk device.

Your feedback:
- If you have any idea why the unmatched and bpif3 boards intermittently fail usertests, please create an "issue" documenting it or pull request if you have any solutions or enhancements to share. Thank you!








