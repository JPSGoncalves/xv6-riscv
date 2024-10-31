# Contents of this Branch

This branch captures changes made to support a sifive-unmatched board running opensbi (its default configuration).

The default xv6 release supports machine mode (only) so this would not run on unmatched without replacing its Uboot-SPL

While that may be a useful exercise, this approach allows a user to run/debug a stock/unmodified unmatched board
or (hopefully) any board that runs uboot (with sbi). 

The other problem with messing with the SPL is it contains all the board (e.g. DDR ram) initialization code. So, effectively,
what we are doing here is interrupting Linux boot by pressing a key while it is in uboot and loading this image
instead of Linux. Either run in RISCV supervisor mode and make use of the memory resident sbi that uboot and Linux uses.

The modifications are fairly minor with a define selection in Makefile and a corresponding one in param.h. A second define is
used for unmatched specific changes:

- SUPPORT_SBI -- enables branch specific changes relating to OpenSBI
- SUPPORT_UNMATCHED -- enable any unmatched specific changes (which may not be essential but used while debugging).

A single define selection is all that is needed but not sure if VS code can properly notice the one in Makefile to help with
its code highlighting features.

# Current Limitations

## Use of Ramdisk

This build borrows a concept from Michael Engel's VF2 port (https://github.com/michaelengel/xv6-vf2) involving its use of
a ramdisk. This simplifies the hardware driver requirement (temporarily) so no need to interface with spi or other peripherals
for now. Of course, there is no persistence, but its fairly easy to rebuild the image with code changes in experimentation.

Dr. Engel's version uses a FAT ramdisk which was harder to port and led to many more differences from the MIT codebase, so this
version simply uses a ramdisk version of the default filesystem that gets created by the original Makefile. I add a utility to
generate ram_disk.h which is included by ram_disk.c that provides the block r/w functionality but does not change any OS caching
complexities.

## Use of sbi console for UART

This build also cheats by avoiding the implementation of a UART driver (for now). This works fine for output but for input
we must poll the SBI API for any characters. Since the clock interrupts are coming in 10x per second, we just poll SBI for
any waiting character within the clock interrupt and push any into the console buffer if found. The hardware buffers a few
characters so even if one types faster than 10x per second they should not be lost.

Later, I made a minor SUPPORT_UNMATCHED efficiency change to simply look at the uart register status directly. This was part
of a debug process where I was trying to cut back on SBI calls. The right way to do that is to adapt the MIT Uart driver
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
- when its moved from its normal compiled address (!) even slightly. I'm including a modified version of usertests
called usertests2 that simply reverses the order of two tests (sbrkmuch and sbrkbasic) and it passes all the time.
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

# Debugging

I'm a big fan of Jtag, so I use riscv64-unknown-linux-gnu-gdb with target remote session to openocd. This works well for

- code load of the kernel elf via Jtag
- setting breakpoints anywhere in the kernel and inspecting kernel variables at the breakpoint.

But gdb doesnt work well for user code debugging. For one thing, user code runs using virtual memory remapping and for another
the paging remapping protects against writing to code space which gdb does (TODO: try modifying xv6 to allow writing to user code space,
maybe gdb can then set breakpoints and inspect user variables?). There is no need for gdb to load the user code itself, just the
user code symbols which is possible to do.

So far, I've use the openocd telnet debugger to examine registers and memory when debugging user code via Jtag and leave gdb in kernel space.

So the standard debug routine is as follows:

Run putty on ttyusb1 at 115200 for terminal (same as terminal debug for linux)

- power board to boot unmatched to its uboot prompt (press a key to stop it launching Linux)
- launch openocd (which halts uboot)

$ openocd -f openocd-jlink.cfg

- launch telnet 

$ telnet localhost 4444 to estabilsh a debug window for user code

- launch gdb 

$ riscv64-unknown-linux-gnu-gdb kernel

(gdb) target extended-remote :3333 

Critical: choose the active thread. This is the one that opensbi was running uboot on and is random. I use the gdb command:

(gdb) info threads

to tell which thread was running uboot. Its the only thread with address 0xfffxxxxx. The others are still in opensbi at 0x8000xxxx.

(gdb) thread X

where X is the numeric thread identified above. Critically, this thread will have a valid 'tp' that identifies the running HART.

Its a pain to identify the running HART at xv6 launch so this code continues to rely on that to be true. Uboot does this by
default when running from its command prompt so it should be possible to boot xv6 binary without a debugger. But if necessary,
the xv6 initialization *can* tell the running hart by querying sbi for which HARTs are busy and only one should be busy at init
time. I may add this later to be independent of this requirement.

Then

(gdb) cont

And xv6 should appear on the terminal

xv6 kernel is booting  
  
hart 2 starting  
hart 4 starting  
hart 1 starting  
init: starting sh  
$  

## OpenOCD debugging

The telnet window can be used to set hardware breakpoints, examine registers and memory via its commands. I use:

- targets (N)  
command to switch threads. Note that numbering is 0 based in Openocd wheras it is 1 based in gdb.
- reg (reg name) 
command to examine and change registers (e.g. reg sepc, reg satp)
- mdh, mdd, mdb  
commands to dump memory (32 bit, 64 bit, 8 bit)
- resume  
command to continue processing. Note that I prefer using gdb's 'cont' command so that it stays in sync with the code better
- bp 0x(address) N hw  
command sets a hardware breakpoint at address range address..address+N (I think). I usually set N to the
byte size of the instruction that I want to break on

Note that it is possible to "look at" user contexts even after stopping at a kernel breakpoint by setting the thread's satp register
to the user satp (visible in the xv6 process structure). Be sure to format it correctly with the ms nibble as '8' and omit the lower
3 nibbles (000) or 12 bits since the satp is a page address. I generally save the kernel satp in case I want to resume a
kernel breakpoint after looking at some user memory. This is the best way I found for examining user variables since gdb isnt
user friendly (yet).

## GDB debugging

- info threads  
is needed to determine the state of the harts. Note that thread 1 is hart 0, 2 is hart 1 etc.
- thread N  
 is needed to switch thread context
- break label  
is used to set a break at a label, e.g. break main
- p /x variable  
This one is really useful in user trap.c to print *p and *p->trapframe to examine the process state and user state at the trap.