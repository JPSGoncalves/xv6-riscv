#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define UART_DATA_REG_16550 0
#define UART_STATUS_REG_16550 5
#define RXDATARDY_MASK_16550 1
#define UNMATCHED_UART_DATA_REG 1

// either uses SBI to poll the console Uart for a character or polls the UART registers direct
// Enable POLL_UART0_DIRECT if SBI polling doesnt work (e.g. latest VF2 SBI releases)
// Although a hack, its also more efficient than SBI since there is no need to transition out of user mode.
void 
sbi_poll_uart0(void)
{
#ifdef POLL_UART0_DIRECT
  volatile int *pUart0 = (volatile int *) UART0;
  #if defined(BOARD_VF2) || defined(BOARD_BPIF3)
    if (pUart0[UART_STATUS_REG_16550] & RXDATARDY_MASK_16550) {
      int ch = pUart0[UART_DATA_REG_16550] & 0xff;
      consoleintr(ch);
    }
  #elif defined(BOARD_UNMATCHED)
    int ch = pUart0[UNMATCHED_UART_DATA_REG];
    // unmatched board more efficiently flags a received character by clearing the sign bit
    if (ch >= 0) {
      ch = ch & 0xff;
      consoleintr(ch);
    }
  #else
    #error "Must define BOARD_UNMATCHED or BOARD_VF2 when using POLL_UART0_DIRECT"
  #endif
#else
  int ch = sbi_console_getchar();
  if (ch > 0) {
    consoleintr(ch);
  }
#endif
}
