/*
 * cli_cfg.h
 *
 * Copyright (c) 2021 Wesley Becker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef CLI_CFG_H_
#define CLI_CFG_H_

#include "S32K148.h"

#define RX_REG_ADDR (&LPUART1->DATA); // address of UART receive buffer here
#define TX_REG_ADDR (&LPUART1->DATA); // address of UART transmit buffer here

// +++ Very specific, better left out of template +++
// bits in register UARTx->STAT
#define UART_TXIR (1<<23)
#define UART_RXIR (1<<21)

/* NVIC Regs and Bits - Check page RM pg. 117 (7.2.3 Determining the bitfield and register location for
configuring a particular interrupt)*/
#define UART1_IRQ 33
#define UART1_IRQ_REG (UART1_IRQ / 32)
#define UART1_IRQ_BIT (1<<(UART1_IRQ % 32))
#define UART1_IRQ_IP_BIT (1<<(8*(UART1_IRQ % 4)+4))
// \+++ Very specific, better left out of template +++

int CliInitUart(void);

void CliDisableUartInt(void);
void CliEnableUartInt(void);

#endif /* CLI_CFG_H_ */
