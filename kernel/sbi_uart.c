#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define VF2_UART_DATA_REG 0
#define VF2_UART_STATUS_REG 5
#define VF2_RXDATARDY_MASK 1
#define UNMATCHED_UART_DATA_REG 1

// either uses SBI to poll the console Uart for a character or polls the UART registers direct
// Enable POLL_UART0_DIRECT if SBI polling doesnt work (e.g. latest VF2 SBI releases)
// Although a hack, its also more efficient than SBI since there is no need to transition out of user mode.
void 
sbi_poll_uart0(void)
{
#ifdef POLL_UART0_DIRECT
  volatile int *pUart0 = (volatile int *) UART0;
  #if defined(BOARD_VF2) && defined(BOARD_UNMATCHED)
      #error "Choose either BOARD_VF2 or BOARD_UNMATCHED, not both"
  #endif
  #if defined(BOARD_VF2)
    if (pUart0[VF2_UART_STATUS_REG] & VF2_RXDATARDY_MASK) {
      int ch = pUart0[VF2_UART_DATA_REG] & 0xff;
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
