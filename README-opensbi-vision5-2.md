# Notes about this branch and StarFive Vision Five 2 (VF2) board

Owing to StarFive use of SiFive U74 cores in the JH7110, the board shares many characteristics with the SiFive unmatched board.

The main difference is the memory map and peripherals surrounding the core.

The xv6 software therefore only has minor differences from Unmatched:

- The clock counter advances faster than unmatched (and is therefore compensated)
- The UART is mapped at a different address and uses a different style interface
- DDR, Uboot and OpenSBI exist at different addresses (from unmatched) but xv6 can run at the same 0x80400000 address since DDR
exists there.

For reference, the VF2 board memory map is described in the JH7110 "Technical Reference Manual" 

```
https://doc-en.rvspace.org/JH7110/TRM/JH7110_TRM/system_memory_map.html
```

The good news is that the usertests program runs without error (the puzzling sbrkmuch test does not seem to fail) on VF2 hardware.

# Jtag connection

I used an inexpensive FT2232H-56Q ftdi minimodule (Digikey 768-1278-ND) as a Jtag probe adapter. It lists for $29.95 in 2024.

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

Please be sure to confirm the pinout of any adapter or cabling with the vendor documentation before making any connections since improper
interconnection can damage the board, interface or (unlikely) the host PC.

# Errata

## SD Card harware issue

My board sometimes doesnt like to boot from the SD card. I havent figured this out yet. Sometimes I disconnect the USB Uart or Jtag from the
host while powering the board and then reconnect as the system is booting. Sometimes I reinsert the SD card.

It could be the particular model of SD card. The board always seems to boot from internal SPI (with the older firmware), but I often run with
the newer firmware on SD in order to verify the UART workaround (see next section).

## OpenSBI console input API

Recent firmware seemed to break this API. Older SBI/Uboot originally loaded on the board supports it fine. As a result, I
added a POLL_UART0_DIRECT option in param.h to avoid this API and poll the Uart Rx logic directly.

This older version (on my board, in SPI flash) works fine for SBI or POLL_UART0_DIRECT:
```
U-Boot SPL 2021.10 (Feb 12 2023 - 18:15:33 +0800)
```

The newer version (on SD) reports an error every time a key is pressed (when using SBI or NOT using POLL_UART0_DIRECT)

```
U-Boot SPL 2021.10 (Sep 19 2024 - 15:43:53 +0800)
```

This is the error message that gets printed when POLL_UART0_DIRECT is not enabled on the newer firmare running on the VF2 board and a key is pressed:

```
$ sbi_ecall_handler: Invalid error 13 for ext=0x2 func=0x0
sbi_ecall_handler: Invalid error 13 for ext=0x2 func=0x0
```

## Example Jtag debug session

This is also covered in the README-jtag-common file but below includes an actual debug session.

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
Example run Within gdb:

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

Prior to running openocd and issuing the 'cont' command, insure that uboot was interrupted and not allowed to boot Linux. It should be at the uboot prompt.

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





