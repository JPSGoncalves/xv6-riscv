# SiFive Unmatched B observations

Refer to the README-opensbi-overview for background on the port that is common for all boards.

This document focuses on the Unmatched board specifics.

The Unmatched pboard is good because is well documented and, owing to SiFive being the manufacturer, a sort of
standard platform that others build from or understand. However, it is more expensive than the other boards supported here.

The board is very stable and nicely fits into a standard ATX case which makes it fairly safe for software development.

OpenSBI appears well behaved and keeps the unused harts at a WFI awaiting sbi API call to start them in main(). It is possible to
compile the provided open source UBoot (SPL and app) and Linux and get it all to work.

# Load address

For unmatched, Xv6 is linked at address 0x80400000. This approximately where Linux is otherwise first loaded so does not interfere with the 
memory resident opensbi code running at 0x80000000.

# Errata

Here I list specific issues that you may encounter with the Unmatched board.

## USB Jtag connection (workaround provided here)

The board has a USB interface that has two ports, one for debug UART that we use and the other for Jtag. Unfortunately, I could not get the USB Jtag interface to be stable. There is an alternate Jtag header port (which also requires a jumper to enable) and a Jlink EDU probe that I happened to own. I have an open ticket with SiFive on the issue and they are investigating (with confirmation that this applies to some boards). 

On Unmatched Rev B, the alternate Jtag header is J6 and the "Jtag mux Sel" jumper is J18 (normally not jumpered). Unfortunately, J6 is a small form factor 0.05" header and this requires an adapter to mate with 0.1" pins on the Jlink probe. I used Olimex ARM-JTAG-20-10 (Digikey 1188-1016-ND at ~ $5.45 in 2024).

BE VERY CAREFUL with this alternate Jtag port! That interface runs at 1.8v and the cheap Jlink EDU now available DOES NOT work at 1.8v - it is
fixed at 3.3v. So you must either use the older Jlink EDU, the professional Jlink or some alternate adapter that supports 1.8v VRef on the interface.

Please be sure to confirm the pinout of any adapter or cabling with the vendor documentation before making any connections since improper
interconnection can damage the board, interface or (unlikely) the host PC.

If the default USB Jtag interface works for you, a slightly different Jtag .cfg file is needed. Its the one in the siFive SDK.

## usertests

One executable in the ramdisk is called usertests. It samples the xv6 kernel API and prints dialog on its progress. The tests
are cleverly written with expected results (including for faults). 

Unfortunately, I've met my match with the intermittent failure of one particular test on unmatched: sbrkmuch() within usertests.c. This test runs fine:

- on the VF2 board
- occasionally (unmodified). Run the test a few times in a row from the command prompt.
- when run standalone (via command line parameter to just run sbrkmuch).
- when its moved or called differently from its original version. Please try included "usertests2" which simply reverses the call order of sbrkbasic and sbrkmuch with no other changes. Both versions are on the supplied ramdisk.
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

# Other Unmatched Comments (not xv6 related)

I dont use a graphics card with my unmatched. 

When I run Linux, I use the network interface to either ssh in or run X2Go server within Linux and X2Go Client on my remote machine. For the server side, I choose openbox which runs well with X2Go and I can live with it.

X2Go client gives me a full X11 graphics interface and its reasonably fast. 

I've successfully compiled and installed Uboot SPL and UBoot to the onboard SPI Flash. So no need for the SD card to get into Linux unless SPI gets corrupted. 

## Debian Linux setup on Unmatched

When not running xv6, I'm running Debian Linux installed on a NVMe SSD. 

If there is interest, I'll try to follow up with info on how all of that got
set up.

What I can say is that it was surprisingly easy to get Debian (SID) running with its vast prebuilt packages available. The
only downside presently is that since RISCV is only supported with their SID (experimental) release its hard to reference
a particular stable release in case the latest has some problem.

## Jtag curiosities with Linux

Its cool to start Jtag while running Linux. I can easily see whats happening under the hood. For example, Realtime Hart 0 is happily
still running in OpenSBI (at a wfi instruction) and the 4 other harts are doing all kinds of stuff with their own page tables. Debugging Linux
at this level is an artform given it runs with its own non unity mapped page tables so its a bit more challenging to
see where it is in the code but thats maybe something for another time. If one hangs out too long at a breakpoint, Linux
gets upset upon resumption with various watchdogs killing the process.Linux maintains time of day, so its certainly not counting interrupts to tell the time (duh). Surprisingly, ssh sessions remain connected for a long time at a Jtag breakpoint
unless you try to type into the terminal session. Then it knows something is up.

