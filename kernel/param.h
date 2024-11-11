#ifndef __PARAM_H
#define __PARAM_H

#ifndef RUNTIME_SBI
// RUNTIME_SBI may be defined in the Makefile so protect from redefinition
#define RUNTIME_SBI
#endif

#ifdef RUNTIME_SBI
#define NPROC        50  // maximum number of processes
#define NCPU          5  // maximum number of CPUs (0 is not used but need to account for it)
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       2000  // size of file system in blocks (for ramdisk. TODO: tie to fs_img_len/BSIZE)
#define MAXPATH      128   // maximum file path name
#define USERSTACK    1     // user stack pages
#define BSIZE        1024  // block size

// SBI default uses API for console input. However
// POLL_UART0_DIRECT is needed on VF2 board due to SBI bug (polls Uart register directly)
// The scheme also works on UNMATCHED board, but select proper BOARD to remap hardware

#define POLL_UART0_DIRECT

// Choose either BOARD_VF2 or BOARD_UNMATCHED but not both else #error
//#define BOARD_UNMATCHED
#define BOARD_VF2

// The following enforces either-or seletion of board (else redefined error)
#ifdef BOARD_VF2
#define TIMERINTCNT  400000 // counter increment until next interrupt -- JH7110
#endif

#ifdef BOARD_UNMATCHED
#define TIMERINTCNT  100000 // counter increment until next interrupt
#endif

#else
#define NPROC        64  // maximum number of processes
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       2000  // size of file system in blocks
#define MAXPATH      128   // maximum file path name
#define USERSTACK    1     // user stack pages
#endif
#endif