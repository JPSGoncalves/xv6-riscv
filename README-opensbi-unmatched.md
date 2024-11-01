# Contents of this Branch

This branch captures changes made to support a sifive-unmatched board running opensbi (its default configuration).

The default xv6 release supports machine mode (only) so this would not run on unmatched without replacing its Uboot-SPL

While that may be a useful exercise, this approach allows a user to run/debug a stock/unmodified unmatched board
or (hopefully) any board that runs uboot (with sbi). 

The other problem with messing with the SPL is it contains all the board (e.g. DDR ram) initialization code. So, effectively,
what we are doing here is interrupting Linux boot by pressing a key while it is in uboot and loading this image
instead of Linux. Either run in RISCV supervisor mode and make use of the memory resident sbi that uboot and Linux uses.

The modifications are fairly minor with a define selection in Makefile and a corresponding one in param.h. 

- RUNTIME_SBI -- enables branch specific changes relating to OpenSBI  

# Quick Start

Interrupt the Unmatched boot process by pressing a key on the USB UART debug terminal while it is in Uboot. This starts
the uboot command interface where it is possible to load and run a binary file (the xv6 kernel binary given as a release or
produced via recompile of the source code).

For example, my 'kernel.bin' file is in my home directory on my nvme drive. Uboot allows me to reference it referenced
to the root of nvme device 0. The unmatched.ld linker file begins the text segment at 0x80400000.

At uboot command prompt:  
```
=> load nvme 0 0x80400000 home/raykj/kernel.bin

2078808 bytes read in 2 ms (991.3 MiB/s)
```
Now, tell uboot to start running at _entry which is at 0x80400000:
```
=> go 0x80400000
```
If you dont have an nvme ssd drive, then you can reference a fat32 file on the SD card. The load command is different:

At uboot command prompt:
```
=> fatload mmc 0:3 0x80400000 kernel.bin
```
mmc manages the SD card and the fat32 partition is (usually partition 3)

then issue the go command as before:
```
=> go 0x80400000
```

This is essentially the same way that Linux boots, except instead of (automatically) loading the Linux kernel file, we 
manually command Uboot to load the xv6 file at a particular address and then run at that address. 

It should be possible to modify uboot scripts to automatically load xv6 file and run it if desired. Another option is
to replace the uboot run time with xv6 instead of uboot but that doesnt really gain us anything. Of possible interest is
to one day replace the uboot spl image with xv6 so that native xv6 could start in machine mode, but that requires porting all
the platform initialization code and, while interesting, is distracting to my goals here.

For me, the interest is in better understanding the RISCV architecture, especially the privileged part relating to Virtual 
Memory and trapping (you can do anything in a trap service routine, even extend the instruction set!).

The real fun comes from building the code, making changes to it and debugging at a source code and low level (please see
the debug section).

I picked the Unmatched platform because it is well documented and, owing to SiFive being the manufacturer, a sort of
standard platform that others build from or understand. Plus, they recently (Sept 2024?) reduced the price to $299 owing to their
P550 platform release ($599 at time of publication). That price is high but you get a lot with this board, although what
you dont get is any kind of gaming performance!

# Current Limitations

## Use of Ramdisk

This build borrows a concept from Michael Engel's VF2 port (https://github.com/michaelengel/xv6-vf2) involving its use of
a ramdisk. This simplifies the hardware driver requirement (temporarily) so no need to interface with spi or other peripherals
for now. Of course, there is no persistence, but its fairly easy to rebuild the image with code changes in experimentation.

Dr. Engel's version uses a FAT ramdisk which was harder to port and led to many more differences from the MIT codebase, so this
version simply uses a ramdisk version of the default filesystem that gets created by the original Makefile. I add a utility to
generate ram_disk.h which is included by ram_disk.c that provides the block r/w functionality but does not change any OS caching
complexities.

The VF2 port also has more extensive differences from the latest xv6 official release.

## Use of sbi console for UART

This build also cheats by avoiding the implementation of a UART driver (for now). This works fine for output but for input
we must poll the SBI API for any characters. Since the clock interrupts are coming in 10x per second, we just poll SBI for
any waiting character within the clock interrupt and push any into the console buffer if found. The hardware buffers a few
characters so even if one types faster than 10x per second they should not be lost.

The right way to do the console that is to adapt the xv6 Uart driver
(16550 based) to use the Unmatched hardware (sifive IP). The Sifive hardware is much simpler so this should be easy but not yet
done in this version.

# Errata

Here I list stuff that I have not yet resolved

## An oddity with SMP

This version launches all 4 U74 cores (Harts) and they seem functional. But I'm not yet sure if there is a problem. I would have expected
the cores to be at a "wfi" instruction awaiting an interrupt (keypress or timer) when idle. I believe the initial core
does wait at wfi but the other ones seem to hang out in r_sstatus() when I debug so thats another thread to pull on.

Note that, like Linux, the S7 core (hart 0) is left at a wfi in OpenSBI. This core does not have supervisor support and therefore can not
run Xv6 or Linux. But it is sometimes useful to peek at unmapped physical memory from OpenOCD (while debugging).

## usertests

One executable in the ramdisk is called usertests. It samples the xv6 API and prints dialog on its progress. The tests
are cleverly written with expected results (including for faults). 

I've met my match with the failure of one particular test on unmatched: sbrkmuch() within usertests.c. This test runs fine:

- occasionally (unmodified). Run the test a few times in a row from the command prompt.
- when run standalone (via command line parameter to just run sbrkmuch).
- when its moved or called differently from its original version.

Please refer to usertests2 which reverses the call order of sbrkbasic and sbrkmuch with no other changes. Both versions are on the supplied ramdisk.

- of course, in the original MIT QEMU emulation.

I've tried everything I can think of to debug this and am close to saying there must be some unpublished Fu740 fault involving
cache or virtual memory or something.

The failure is consistent in sbrkmuch but inconsistent in actual failure. Sometimes its an illegal instruction, sometimes a page fault.
The errors occur at different addresses and these seem to shift around when the code is slightly modified. I was convinced there
was a problem with interrupt handling that would explain the variabilty but I modified my sbi keyboard polling hack to slow down
interrupts by 100x when a particular key was pressed (e.g. during the test as we approach sbrkmuch) but this had no effect (!) on
the fault. Sometimes it faults in the run() function just prior to running the sbrkmuch().

BTW, I've found that its very difficult on this chip to tell how it got to a particular address. I dont believe it has any PC trace
buffer so I've resorted to leaving breadcrumbs and cycling thru (user and sometimes kernel) code changes and rebuilds to conclude
that I cant seem to figure it out.

xv6 nicely traps the error and never crashes however.

# Debugging

I'm a big fan of Jtag, so I use riscv64-unknown-linux-gnu-gdb with target remote session to openocd. This works well for

- code load of the kernel elf via Jtag
- setting breakpoints anywhere in the kernel and inspecting kernel variables at the breakpoint.

But gdb doesnt work well for user code debugging. For one thing, user code runs using virtual memory remapping and for another
the paging remapping protects against writing to code space which gdb does (TODO: try modifying xv6 to allow writing to user code space,
maybe gdb can then set breakpoints and inspect user variables?). There is no need for gdb to load the user code itself, just the
user code symbols which is possible to do.

I use the openocd telnet debugger (on port 4444) to examine registers and memory when debugging user code via Jtag and leave gdb in kernel space.

So the standard debug routine is as follows:

Run putty, serial, on ttyusb1 at 115200 for a terminal (same as terminal debug for linux).

- power board to boot unmatched to its uboot prompt (press a key to stop it launching Linux)
- launch openocd (which halts uboot)
```
$ openocd -f openocd-jlink.cfg
```
- launch telnet 
```
$ telnet localhost 4444 to estabilsh a debug window for user code
```
- launch gdb 
```
$ riscv64-unknown-linux-gnu-gdb kernel

(gdb) target extended-remote :3333 
```
Critical: choose the active thread. This is the one that opensbi was running uboot on and is random. I use the gdb command:
```
(gdb) info threads
```
to tell which thread was running uboot. Its the only thread with address 0xfffxxxxx. The others are still in opensbi at 0x8000xxxx.
```
(gdb) thread N
```
where N is the numeric thread identified above. Critically, this thread will have a valid 'tp' that identifies the running HART.

Its a pain to identify the running HART at xv6 launch so this code continues to rely on that to be true. Uboot does this by
default when running from its command prompt so it should be possible to boot xv6 binary without a debugger. But if necessary,
the xv6 initialization *can* tell the running hart by querying sbi for which HARTs are busy and only one should be busy at init
time. I may add this later to be independent of this requirement.
```
(gdb) load
```
This actually loads the kernel elf into memory, initializing all of the segments (no bss zero initialization which xv6 seems
to depend on, so I added a bss clear at init time).

Then
```
(gdb) cont
```
And xv6 should appear on the terminal:
```
xv6 kernel is booting  
  
hart 2 starting  
hart 4 starting  
hart 1 starting  
init: starting sh  
$  
```
## OpenOCD debugging

The telnet window can be used to set hardware breakpoints, examine registers and memory via its commands. I use:
```
targets N  
```
command to switch threads. Note that numbering is 0 based in Openocd wheras it is 1 based in gdb.
```
reg (reg name) 
```
command to examine and change registers (e.g. reg sepc, reg satp)
```
mdh, mdd, mdb  
```
commands to dump memory (32 bit, 64 bit, 8 bit), e.g. 
```
mdh 0x80400000 16 
```
dumps the 1st 16 32-bit words of the kernel code )depending on page tables).
```
resume  
```
command to continue processing. Note that I prefer using gdb's 'cont' command so that it stays in sync with the code better
```
bp 0x(address) N hw  
```
command sets a hardware breakpoint at address range address..address+N (I think). I usually set N to the
byte size of the instruction that I want to break on

Note that it is possible to "look at" user contexts even after stopping at a kernel breakpoint by setting the thread's satp register
to the user satp (visible in the xv6 process structure). Be sure to format it correctly with the ms nibble as '8' and omit the lower
3 nibbles (000) or 12 bits since the satp is a page address. I generally save the kernel satp in case I want to resume a
kernel breakpoint after looking at some user memory. This is the best way I found for examining user variables since gdb isnt
user friendly (yet).

On my unmatched board, I was unable to use the default USB Jtag interface. The Unmatched USB has 2 ports: UART and Jtag.
The UART port is on ttyUSB1 (Debian) and works fine. But I could not get a stable connection on the Jtag port using any openocd config
that I could find or edit. I opened a ticket on this with SiFive and got confirmation that the problem was repeatable. But no
solution. So luckily, the board has a 2nd Jtag port that can be accessed with a jumper connection. But BE CAREFUL this port
requires the debugger interface to work at 1.8v and not all cheap adapters either observe the 'VTRef' pin (set to 1.8v on
unmatched or they only run at 3.3v or worse). I have an older Jlink 'EDU' model that does work at 1.8v and does observe the
VTRef pin. But Segger's replacement EDU model is hardware strapped to 3.3v according to documentation that I've read. If so,
this wil likely fry the unmatched Jtag mux (or worse). This is unfortunate because the Segger EDU adapter is otherwise
reasonably priced.

## GDB debugging
```
info threads  
```
is needed to determine the state of the harts. Note that thread 1 is hart 0, 2 is hart 1 etc.
```
thread N  
```
is needed to switch thread context
```
break label  
```
is used to set a break at a label, e.g. break main
```
p /x variable  
```
This one is really useful in user trap.c to print *p and *p->trapframe to examine the process state and user state at the trap.

# Changing the user programs

Please refer to the README in mk-ramdisk for information on how to update the ram_disk.h file which contains all of the user programs as a memory image.

# Non XV6 stuff

## Uboot tips

help is your friend - it shows all the available commands. In the quick start section, I used a couple of them and, in 
addition to getting a full list of commands you can ask for help on any of them individually. For example:
```
=> help fatls  
fatls - list files in a directory (default /)  

Usage:  
fatls <interface> [<dev[:part]>] [directory]  
    - list files from 'dev' on 'interface' in a 'directory'  
```
So to check that you have the right parameters for load, you can do a fatls first:  
```
=> fatls mmc 0:3  
   247608   config-6.10.6-riscv64  
            extlinux/  
    11115   hifive-unmatched-a00.dtb  
 38485527   initrd.img-6.10.6-riscv64  
            lost+found/  
       83   System.map-6.10.6-riscv64  
 26887168   vmlinux-6.10.6-riscv64  
   540720   kernel  
   303632   kernel.bin  

7 file(s), 2 dir(s)  
```
Here, you can see a (random) kernel.bin that I had on the root partition of a bootable SD card. I also had the kernel
elf file but the bin file works fine here (no advantage for debug symbols without the Jtag debugger).

## Misc Unmatched setup

I dont use a graphics card with my unmatched. Its not a gaming platform, or at least I'm not using it for that.

When I run Linux, I use the network interface to either ssh in or run X2Go server within Linux and X2Go Client on my remote machine. For the server side, I choose openbox which runs well with X2Go and I can live with it.

X2Go client gives me a full X11 graphics interface and its reasonably fast.

I've installed Uboot SPL and UBoot to the onboard SPI Flash. So no need for the SD card to get into Linux unless SPI gets corrupted. These have remained untouched through all of my Xv6 debug sessions by virtue of using the provided SBI facilities
and Uboot to either launch XV6 or its (supervisor) Jtag environment to load into and run from there. 

Starting from uboot which is already in supervisor mode, Xv6 doesnt need code to transition
from machine mode to supervisor mode. There are 'enough privs' enabled by uboot to do anything we need memory, interrupt
or IO wise. 

My thinking here is that if Linux can run following uboot, so can we!


## Debian Linux setup on Unmatched

When not running xv6, I'm running Debian Linux installed on a NVMe SSD. 

If there is interest, I'll try to follow up with info on how all of that got
set up.

What I can say is that it was surprisingly easy to get Debian (SID) running with its vast prebuilt packages available. The
only downside presently is that since RISCV is only supported with their SID (experimental) release its hard to reference
a particular stable release in case the latest has some problem.

## Jtag curiosities with Linux

Its cool to start Jtag while running Linux. I can easily see whats happening under the hood. For example, Hart 0 is happily
still running in OpenSBI (at a wfi instruction) and the 4 other harts are doing all kinds of stuff with their own page tables. Debugging Linux
at this level is an artform given it runs with its own non unity mapped page tables so its a bit more challenging to
see where it is in the code but thats maybe something for another time. If one hangs out too long at a breakpoint, Linux
gets upset upon resumption with various watchdogs killing the process.Linux maintains time of day, so its certainly not counting interrupts to tell the time (duh). Surprisingly, ssh sessions remain connected for a long time at a Jtag breakpoint
unless you try to type into the terminal session. Then it knows something is up.

