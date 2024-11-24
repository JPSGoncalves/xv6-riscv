# Notes about this branch and Banana Pi F3 (bpif3) board

This board is quite different from unmatched and VisionFive 2 boards. Its SOC is a SpaceMit-k1 Octocore and has a
completely different set of peripherals and memory map.

The board does use a similar boot sequence to unmatched/vf2:

- execution starts in the SOC proprietary ROM
- Control is transferred to a SPL that sets up the hardware
- Control is transferred to Uboot which contains a memory resident SBI
- Control is transferred to Linux if Uboot is not interrupted by the user

The executables for the non ROM components can reside on multiple non volatile memory devices (I've mainly used the provided
SD card image so far).

Although source code is available for all but the ROM part, I've so far been unable to replicate the distributed image.

The best I've been able to do is replace Uboot+SBI. When I compile the SPL code, it fails to load due to some
cryptographic failure. From this, I conclude that the SPL image is signed and therefore not really user replacable. Maybe
this is buried somewhere in the vendor full SDK?  

Also the self compiled uboot seems to have a lot of debug material printed that I dont see in the vendor provided image.  

Most concerning to development is that although it has a Jtag interface, RISCV cores 1-7 are not visible prior to Linux booting. I
think this SOC has some clever core power down registers that must be initialized to bring them online, but I have not yet decoded
the magic sequence yet.

In summary, the observed differences beween bpif3 and other boards:

- The clock counter advances much faster than the other boards.
- The UART is mapped at a different address (have not tried to interface to it directly yet)
- DDR, Uboot and OpenSBI exist at different addresses. Linux appeared to be loading around 0x10000000, so xv6 is linked
to locate starting at this address.
- It has 8 cores, but I have been unable to bring them all online (only 4 appear to start when requesting them from SBI using the
published API, with 4, 5 and 6 seemingly missing in action :).
- As mentioned, Jtag is not available for cores 1-7 until the SOC is executing somewhere in Linux.
- The SPL boot section appears to be cryptographically signed.
- The realtime core runs some ".elf" file when booting Linux. I can not find the source to this .elf file.  

By selecting bpif3 in the Makefile, an image is created that accounts for the differences. But the user must remember to load the kernel.bin file to 0x10000000 if not using the kernel elf file (with gdb).

# Jtag connection

I used an inexpensive FT2232H-56Q ftdi minimodule (Digikey 768-1278-ND) as a Jtag probe adapter. It lists for $29.95 in 2024.

## FT2232H-56Q minimodule Jtag wiring details

```
GPIO Pin, Signal, MiniModule pin

     1    VIO    CN2-12
     7    TDI    CN2-9
    11    TMS    CN2-11
    13    TCK    CN2-8
    15    TDO    CN2-10
    25    gnd    CN2-1
    n/c   nTRST  CN2-14 
```
Where:

- TMS, TCK, TDI, TDO are normal Jtag signals
- VIO is a voltage reference from bpif3 to the mini module (3.3v)
- gnd is 0v 
- nTRST is a GPIO on the FT2232H but not connected to bpif3. That pin is hard wired on the board to the reset circuit and
generally gets reset once. As a result, the config file setting for this pin doesnt matter.

Please be sure to confirm the pinout of any adapter or cabling with the vendor documentation before making any connections since improper
interconnection can damage the board, interface or (unlikely) the host PC.

# Errata

## usertests

Fascinatingly, this board fails the usertests program frequently, but sometimes at a new place - rmdot.

```
...
test subdir: OK
test bigwrite: OK
test bigfile: OK
test fourteen: OK
test rmdot: usertrap(): unexpected scause 0x2 pid=19216
            sepc=0x6e26 stval=0x0
FAILED
SOME TESTS FAILED
$
```

It may also fail at sbrkmuch (!) just like unmatched. Curious. And sometimes it passes all tests: 

```
...
test bigdir: OK
test manywrites: OK
test badwrite: OK
test execout: OK
test diskfull: balloc: out of blocks
balloc: out of blocks
OK
test outofinodes: ialloc: no inodes
OK
ALL TESTS PASSED
$
```

I did not do as much detailed debugging with this board as was done on unmatched to look beyond the failure string.

## cores 1-7

Its possible to run xv6 with a single core. In main.c, comment out this:

```
    // harts 0-7 (inclusive)
    //for (int i = 0; i < NCPU; i++)
    //{
    //  if (i != boothartid)
    //  {
    //    sbi_hart_start(i, (unsigned long) &_entry_sbi);
    //  }
    //}
```
That single core will boot xv6 with a single core and jtag works with core 0 and all is good.

The problem is with cores 1-7, which appear inaccessible pre-Linux. Attempts to start them with the above code is partially successful.

I end up with exactly 5 cores running. Those are visible to Jtag once they are running. The others silently fail Jtag for some unknown reason.

# Jtag Workaround Attempt

Given that all 8 cores are visible once we are in Linux, I used the telnet openocd session to try and take control of them and run xv6.

This is the approach:

For all cores,
```
reg satp 0x0
reg sie 0x0
reg tp = N (where N ranges from 1..7)
reg pc = (address of) spin
```

After loading the code with core 0 (thread 1 in gdb), xv6 runs with core 0 and all the rest are waiting at wfi at spin (label in entry_sbi.S).

Next, halt the code and one by one initialize a hart with the hart ID in a0, and set pc to ```_entry_sbi```  

For example, hart 1 can enter xv6 this way (assuming it was previously at the label ```spin```):

```
reg a0=1
reg pc=_entry_sbi
resume
```

After doing this for all harts, they were confirmed running xv6. So xv6 is ok with all 8 cores, but something stuck in SBI to launch them.

I tried to tuck all of the Linux cores back into SBI by first setting their PC to the new function 'park' which calles SBI to "stop" them and then try to run unmodified xv6 that calls SBI to start cores 1..7 at the appropriate time.

That plan failed, although park seems to power down the core (and Jtag cant talk to it once that happens). So thats a clue.

The real solution to this is to get Jtag to work while the code is running Uboot to try and figure out why they cant emerge when requested. Unresponsive cores appear to be "powered off" and the code to turn them on (so maybe they could wait at a wfi like the U74 cores do) is not
documented (I specifically saw some undocumented register addresses getting written to in the SBI source code that power them up).

This code snapshot calls SBI for cores 0-7 (skipping the boot core) and ends up with 5 running cores with no error returned for the cores that dont start (yes, the return value was checked offline).

# Closing Thought

This board has great potential for custom development but I think that is hindered by the lack of clear published information about what is needed to make it work properly. I'm hopeful that more information will be made available or others contribute to the project their solutions (via pull request).

