# Notes about this branch and StarFive Vision 5 (2) or VF2 board

Prior to make, in param.h, select BOARD_VF2 and deselect BOARD_UNMATCHED for the VF2 version.
```
//#define BOARD_UNMATCHED
#define BOARD_VF2
```
In param.h, also select POLL_UART0_DIRECT to poll UART0 for console receive characters within xv6 (rather than use SBI for console polling). 

Early versions of VF2 firmware supported pure SBI but this option sidesteps the issue by talking directly to the UART for console receive characters.

For convenience, the v0.2 release contains both "pure SBI" and "Uart0 direct" binaries (and elf files in case using Jtag). See the README file in the release for additional comments.

- The usertests issues described in the unmatched README do not happen on VF2 board.
- Selecting BOARD_VF2 adjusts the timer interrupt rate to compensate for observed higher timer clock frequency.
- I'm using a different Jtag adapter for the VF2 board since I didnt want to move the Jlink EDU (old version) off of the unmatched yet. I'm specifically using an FTDI FT2232H-56Q minimodule. I'll include a working Jtag config that I'm using in the Jtag directory.

## Errata

As described above, the 'pure sbi' VF2 configuration does not process console keyboard input correctly with recent board firmare.

While it boots, there is something wrong with the keyboard input -- I get an SBI error with each keypress:

```
$ sbi_ecall_handler: Invalid error 13 for ext=0x2 func=0x0
sbi_ecall_handler: Invalid error 13 for ext=0x2 func=0x0
```

I've since traced the source of this error to some difference in boot code on the SD card versus what I was previously running from SPI (boot mode 00, or "Flash" on the PCB). 

Its odd because both versions report the same SPL/SBI version, but the one that works is the older one:

This one works fine:
```
U-Boot SPL 2021.10 (Feb 12 2023 - 18:15:33 +0800)
```

The newer version reports the aforementioned error.

```
U-Boot SPL 2021.10 (Sep 19 2024 - 15:43:53 +0800)
```

The VF2 board has a different DDR memory map from siFive unmatched board but its kernel.bin load address (0x80400000) is still within valid space with plenty of headroom. The code really should be recompiled for something around 0x40xxxxxx where xxxxxx leaves room for SBI that loads at 0x40000000.

I dont know why, but it took a long time to locate the VF2 memory map. It is in the JH7110 "Technical Reference Manual" 

```
https://doc-en.rvspace.org/JH7110/TRM/JH7110_TRM/system_memory_map.html
```


## FT2232H-56Q minimodule Jtag wiring details

```
GPIO Pin, Signal, MiniModule pin

     1    VIO    CN2-12
    35    TMS    CN2-11
    36    nTRST  CN2-14
    37    TCK    CN2-8
    38    TDI    CN2-9
    39    gnd    CN2-1
    40    TDO    CN2-10
```

Where:

- TMS, TCK, TDI, TDO are normal Jtag signals
- VIO is a voltage reference from the VF2 to the mini module (3.3v)
- gnd is 0v 
- nTRST is a GPIO on the FT2232H and configured in the jtag/openocd-ftdi.cfg file used to invoke openocd

## running openocd

```
openocd -f openocd-ftdi.cfg
```
Do this in one terminal session. In another, do
```
telnet localhost 4444
```
In a 3rd, invoke gdb:
```
riscv64-unknown-linux-gnu-gdb kernel
```
Within gdb:
```
(gdb) target extended-remote :3333
(gdb) info threads
(gdb) thread N
(gdb) cont
```

Where N is the thread currently running uboot with pc address around 0xfffxxxxx

Example run:
```
(gdb) target extended-remote :3333
Remote debugging using :3333
0x0000000040004c50 in ?? ()
(gdb) info threads
  Id   Target Id                                                  Frame 
* 1    Thread 1 "e24.cpu0" (Name: e24.cpu0, state: debug-request) 0x0000000040004c50 in ?? ()
  2    Thread 2 "u74.cpu1" (Name: u74.cpu1, state: debug-request) 0x00000000fff7588e in ?? ()
  3    Thread 3 "u74.cpu2" (Name: u74.cpu2, state: debug-request) 0x0000000040009fb6 in ?? ()
  4    Thread 4 "u74.cpu3" (Name: u74.cpu3, state: debug-request) 0x0000000040004c52 in ?? ()
  5    Thread 5 "u74.cpu4" (Name: u74.cpu4, state: debug-request) 0x0000000040009fb6 in ?? ()
(gdb) thread 2
[Switching to thread 2 (Thread 2)]
#0  0x00000000fff7588e in ?? ()
(gdb) load
Loading section .text, size 0x7000 lma 0x80400000
Loading section .rodata, size 0x7cc lma 0x80407000
Loading section .data, size 0x1f4058 lma 0x804077d0
Loading section .got, size 0x20 lma 0x805fb828
Loading section .got.plt, size 0x10 lma 0x805fb848
Start address 0x0000000080400000, load size 2078804
Transfer rate: 183 KB/sec, 15868 bytes/write.
(gdb) info threads
  Id   Target Id                                                  Frame 
  1    Thread 1 "e24.cpu0" (Name: e24.cpu0, state: debug-request) 0x0000000040004c50 in ?? ()
* 2    Thread 2 "u74.cpu1" (Name: u74.cpu1, state: debug-request) 0x0000000080400000 in _entry ()
  3    Thread 3 "u74.cpu2" (Name: u74.cpu2, state: debug-request) 0x0000000040009fb6 in ?? ()
  4    Thread 4 "u74.cpu3" (Name: u74.cpu3, state: debug-request) 0x0000000040004c52 in ?? ()
  5    Thread 5 "u74.cpu4" (Name: u74.cpu4, state: debug-request) 0x0000000040009fb6 in ?? ()
(gdb) cont
Continuing.

```

Beyond openocd invocation, this is identical to the instructions from the Jtag section of the unmatched readme.

Prior to issuing the 'cont' command, insure that uboot was interrupted and not allowed to boot Linux.

The uboot terminal shows this (starting from power):

```
U-Boot 2021.10 (Feb 12 2023 - 18:15:33 +0800), Build: jenkins-VF2_515_Branch_SDK_Release-24

CPU:   rv64imacu
Model: StarFive VisionFive V2
DRAM:  8 GiB
MMC:   sdio0@16010000: 0, sdio1@16020000: 1
Loading Environment from SPIFlash... SF: Detected gd25lq128 with page size 256 Bytes, erase size 4 KiB, total 16 MiB
*** Warning - bad CRC, using default environment

StarFive EEPROM format v2

--------EEPROM INFO--------
Vendor : StarFive Technology Co., Ltd.
Product full SN: VF7110B1-2253-D008E000-00005412
data version: 0x2
PCB revision: 0xb2
BOM revision: A
Ethernet MAC0 address: 6c:cf:39:00:4d:a4
Ethernet MAC1 address: 6c:cf:39:00:4d:a5
--------EEPROM INFO--------

In:    serial@10000000
Out:   serial@10000000
Err:   serial@10000000
Model: StarFive VisionFive V2
Net:   eth0: ethernet@16030000, eth1: ethernet@16040000
switch to partitions #0, OK
mmc1 is current device
found device 1
bootmode flash device 1
406 bytes read in 4 ms (98.6 KiB/s)
Importing environment from mmc1 ...
Can't set block device
Hit any key to stop autoboot:  0
StarFive #
```
This is where uboot was interrupted. Now, after the 'cont' gdb command we see this:
```
xv6 kernel is booting

hart 4 starting
hart 2 starting
hart 3 starting
init: starting sh
$

```
Now, running usertests succeeds (unlike unmatched)!




