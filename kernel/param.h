#ifndef __PARAM_H
#define __PARAM_H

// This selection moved to Makefile but leaving here when helpful for MS Code editing
//#ifndef BOARD_UNMATCHED
//#define BOARD_UNMATCHED
//#endif
//#define BOARD_VF2
//#define BOARD_BPIF3

//#ifndef RUNTIME_SBI
// RUNTIME_SBI may be defined in the Makefile so protect from redefinition
//#define RUNTIME_SBI
//#endif

#ifdef RUNTIME_SBI
#define NPROC        50  // maximum number of processes
#ifdef BOARD_BPIF3
#define NCPU          8  // maximum number of CPUs
#else
#define NCPU          4  // maximum number of CPUs
#endif
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

// POLL_UART0_DIRECT selection
//
// Board        |   POLL_UART_DIRECT   |   Comment
// -----------------------------------------------------------------------------------------------------
// Unmatched    |   defined            |  Use Uart access for console Rx
// Unmatched    |   not defined        |  Use SBI API for console Rx
// VF2          |   defined            |  Use Uart access for console Rx
// VF2          |   not defined        |  Use SBI API for console Rx. But
//              |                      |  does not work reliably (depends on Starfive firmware version)
// Banana-pi F3 |   defined            |  Not yet supported
// Banana-pi F3 |   not defined        |  Use SBI API for console Rx

//#define POLL_UART0_DIRECT

// The following enforces either-or seletion of board (else redefined error)
#ifdef BOARD_VF2
#define TIMERINTCNT  400000 // counter increment until next interrupt -- JH7110
#endif

#ifdef BOARD_UNMATCHED
#define TIMERINTCNT  100000 // counter increment until next interrupt
#endif

#ifdef BOARD_BPIF3
#define TIMERINTCNT  2400000 // counter increment until next interrupt
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