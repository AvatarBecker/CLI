/*
 * cli_cfg.c
 *
 *  Created on: Dec 29, 2021
 *      Author: avatar
 */

#include "cli_cfg.h"

void CliDisableUartInt(void)
{
    LPUART1->CTRL &= ~0x800000;
    LPUART1->DATA = 0;
    //LPUART1->STAT &= ~UART_TXIR;
}
void CliEnableUartInt(void)
{
    //LPUART1->STAT &= ~UART_TXIR;
    LPUART1->DATA = 0;
    LPUART1->CTRL |= 0x800000;
}

int CliInitUart()
{
    // Remember to configure the clocks for the peripheric

    LPUART1->BAUD = 0x12000016;
    /* Oversampling = 18: 28-24 = b10010
     * Baud = 115200 = fclk/((OSR+1)*SBR) -> SBR=21.9298: 12-0 = b0_0000_0001_0110
     * 0.32% error, waay better
     *
     * fclk = 48MHz;
     */

    LPUART1->CTRL = 0x002C0000;
    /* TIE (Tx Data Register Empty Interrupt Enable): 23 = 1
     * RIE (Receive Complete Interrupt Enable): 21 = 1
     * Tx Enable: 19 = 1
     * Rx Enable: 18 = 1
     */

    // Configure interrupts (remember to config NVIC if using ARM)
    S32_NVIC->ICPR[UART1_IRQ_REG] = UART1_IRQ_BIT;    /* Check page 117 of RM */
    S32_NVIC->ISER[UART1_IRQ_REG] = UART1_IRQ_BIT;

    return 0;
}
