# Instructions for updating a kernel/ram_disk.h file from fs.img

To replace the default ram_disk.h file, do the following:  

Make use of the provided fsimg2c utility to convert a fs.img file to a c header file suitable for compilation.

- compile the (edited) user/ programs (from the project root directory, where the kernel is otherwise compiled)
```
$ make userprogs  
```
This creates fs.img  

- cd into mk-ramdisk, cp the fs.img locally and run fsimg2c  
```
$ cd mk-ramdisk  
$ cp -p ../fs.img .  
$ ./fsimg2c  
```
This creates ram_disk.h  

- Copy the ram_disk.h into kernel  
```
$ cp -p ram_disk.h ../kernel  
```
- Compile the kernel with the new ramdisk  
```
$ cd ..  
$ make  
```

- Run the resulting kernel.bin or use Jtag to load kernel  

Your user program edits should now be compiled and loaded into the kernel ramdisk at boot time.

The use of -p option preserves date in case there is confusion about which compiled user programs will appear in the xv6 ramdisk.