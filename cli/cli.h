/*
 * cli.h
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

#ifndef CLI_H_
#define CLI_H_

#include "bsp.h"
#include <string.h>

/* Steps to use the CLI:
 *     Populate your commands in "commands" in cli_cmds.c;
 *     Provide callbacks for each one (or leave NULL for no action);
 *     Instantiate and initialize tCli in main.c;
 *     Register Rx and Tx ISRs for the UART on the Vector Table, or Call them where appropriate;
 */

#define DEBUG
//#define SUPPORT_CLI_NAVIGATION /* turns off arrows, backspace, delete, home, and end. */
#define NUM_OUT_MSG_QUEUE 50 /* Num of messages that can be queued in CliSendString(). Depends also on the number of commands you have. */
#define NUM_IN_CMD_RECALL 10 /* how many commands should we recall */
#define LEN_STD_STR 30

extern char white_spaces[];
extern const char prompt[];

typedef struct
{
    unsigned *rx_reg_addr;
    unsigned *tx_reg_addr;
    void (*DisableUartInt)(void);
    void (*EnableUartInt)(void);
    volatile char was_input_received;
    int idx;
    int in_buff_idx;
    int len_in_buffer[NUM_IN_CMD_RECALL];   // still to be implemented
    /*char *input_buffer;*/
    char input_buffer[NUM_IN_CMD_RECALL][LEN_STD_STR];
    char *output_buffer[NUM_OUT_MSG_QUEUE];
} tCli; /* up to the user to instantiate*/

typedef struct
{
    char *handle;
    char *description;
    int (*callback)(tCli *p_cli, char*);
} tCmd;

extern tCmd commands[]; /* initialized in cli.c, "NULL" terminated */

int CliInit(tCli*);
int CliDeinit(tCli*);

int CliRxISR(tCli *p_cli); /* ISR for each char received */
int CliTxISR(tCli *p_cli); /* ISR for each "ready to send char" */

char* CliUtoa(unsigned long value, char *str, int base);

void CliSendString(tCli *p_cli, const char *orig); /* Non-blocking send until NULL */
int CliHandleInput(tCli *p_cli); /* Goes through commands (until "NULL") checking if anything matches */

int CliInsertChar(tCli *p_cli, char *str, int position, char character);

// to be implemented

int CliClear(tCli *p_cli);

#endif /* CLI_H_ */

