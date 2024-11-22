
This document summarized commong Jtag debugging techniques that can be used across all Jtag supported boards. The main board
specific step concerns which openocd config file is invoked. Physical connection details are different for each board and
described in the board specific README files.

# Jtag Debugging Setup

In order to support Jtag debugging, the following must be done

- Connect to the target board using an adapter (refer to board specific README for details)
- Install RISCV capable openocd
- Install RISCV capable gdb

I use riscv64-unknown-linux-gnu-gdb with target remote session to openocd. This works well for:

- code load of the kernel elf via Jtag
- setting breakpoints anywhere in the kernel and inspecting kernel variables at the breakpoint.

Note that gdb doesnt work well out of the box for xv6 user code debugging. For one thing, user code runs using virtual memory remapping and for another the paging remapping protects against writing to code space which gdb does.  

TODO: try modifying xv6 to allow writing to user code space, maybe gdb can then set breakpoints and inspect user variables?). There is no need for gdb to load the user code itself, just the user code symbols which is possible to do.

I open both a telnet and debug session to openocd. gdb works best for kernel debugging since it is loaded with kernel symbols and the telnet
session can be used for very low level debugging (see below).

The standard debug routine is as follows:

Run putty on the Debug Uart to monitor boot progress as described in the README-overview file (quick start).

For unmatched and vf2 boards (the bpif3 board has an issue with Jtag at boot time, see board specific README for an imperfect workaround to this):

- stop the booting process at the uboot prompt (press a key to stop it launching Linux). But instead of using ```fatload``` and ```go``` uboot commands, we will use Jtag to load the compiled elf file for the board.
- launch openocd (which halts uboot on unmatched and vf2 boards, referring to the jtag/ subdirectory for the required .cfg files)  

To launch openocd for unmatched:
```
$ openocd -f openocd-jlink.cfg
```
To launch openocd for vf2 board:
```
$ openocd -f openocd-ftdi.cfg
```
To launch openocd for bpif3 board:
```
$ openocd -f spacemit_k1_ftdi.cfg
```
- launch telnet to establish a register and memory level debug window for user code (see below)
```
$ telnet localhost 4444 
```
- launch gdb to load and debug the code
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

The code relies on valid tp for code launched from Uboot. But if necessary,
the xv6 initialization *can* determine the running hart by querying sbi for which HARTs are busy and only one should be busy at init
time. I may add this later to be independent of this requirement.
```
(gdb) load
```
This command loads the kernel elf into memory, initializing all of the segments (no bss zero initialization which xv6 seems
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
## Telnet debugging

The telnet window on port 4444 can be used to set hardware breakpoints, examine registers and memory via its commands. I use:
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
dumps the 1st 16 32-bit words of the kernel code (address depends on board memory map).
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
```
cont
```
Is used to resume or start execution based on the current state of the machine.

