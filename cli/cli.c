/*
 * cli.c
 *
 * MIT License
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

#include "cli.h"

#define LEN_PROMPT 2           // length in printable chars
const char prompt[] = "\r> ";  // update LEN_PROMPT as well

char white_spaces[LEN_STD_STR + LEN_PROMPT] = { 0 };

tCli *p_cli;    // passed by application, which must have allocated it

int CliInit(tCli *p_cli_arg)
{
    p_cli = p_cli_arg;

    p_cli->idx = 0;
    p_cli->was_input_received = 0;

    p_cli->rx_reg_addr = RX_REG_ADDR;
    p_cli->tx_reg_addr = TX_REG_ADDR;

    p_cli->EnableUartInt = CliEnableUartInt;
    p_cli->DisableUartInt = CliDisableUartInt;

    CliInitUart();

    memset(p_cli->output_buffer, 0, NUM_OUT_MSG_QUEUE*sizeof(char*));

    memset(white_spaces, ' ', LEN_STD_STR + LEN_PROMPT);
    white_spaces[LEN_STD_STR + LEN_PROMPT - 1] = 0;

    CliSendString(prompt);

    return 0;
}

void CliPeriodicCheck()
{
    if(p_cli->was_input_received)
    {
        CliHandleInput();
    }
}

char* CliUtoa(unsigned long value, char *str, int base)
{
    char temp_str[33] = { 0 }; // null-term. string for holding up to 64 digits (if bin)
    unsigned digit = 0;
    unsigned num_digits = 0;

    unsigned i = 0;
    for (; i < 33; ++i)
    {
        digit = value % base;
        value /= base;

        if (digit > 9)
        {
            temp_str[i] = 'A' + digit - 10;
        }
        else
        {
            temp_str[i] = '0' + digit;
        }

        if (value == 0)
        {
            break;
        }
    }

    temp_str[++i] = 0;

    num_digits = strlen(temp_str);

    for (unsigned j = 0; j < num_digits; ++j)
    {
        str[j] = temp_str[num_digits - j - 1];
    }

    return str;
}

int CliRxISR()    // ISR for each char received
{
    int retval = 0;
    char rec_char = 0;
    static char esc_number = 0;
    static int last_in_buff_idx = NUM_IN_CMD_RECALL - 1;
    static int history_buff_idx = 0;
    static char stashed_buffer[LEN_STD_STR] = { 0 };

    char str_buffer[4] = { 0 };  // up to 999
    char temp = 0;

    // debug
    static char escape_buff[LEN_STD_STR] = { 0 };
    static int eb_idx = 0;

    static enum
    {
        eNO_ESC_SEQ, eESC_RECVD, eO_RECVD, eBRCKT_RECVD, eESC_NUM, eVT_SEQ
    } esc_state = eNO_ESC_SEQ;

    rec_char = *p_cli->rx_reg_addr;

    switch (rec_char)
    {
        case '\n':
        case '\r':
            temp = p_cli->input_buffer[p_cli->in_buff_idx][0];
            if (temp != 0 && temp != ' ' && temp != '\t')
            {
                p_cli->was_input_received = 1;
                last_in_buff_idx = p_cli->in_buff_idx;
                history_buff_idx = last_in_buff_idx + 1;
                if (history_buff_idx > NUM_IN_CMD_RECALL - 1)
                {
                    history_buff_idx = 0;
                }
            }
            else
            {
                p_cli->input_buffer[p_cli->in_buff_idx][0] = 0;
                p_cli->idx = 0;
                CliSendString("\r\n");
                CliSendString(prompt);
                //CliSendString("\r\n> ");
            }
            break;
        case '\b':
        case 127:
            retval = CliInsertChar(p_cli->input_buffer[p_cli->in_buff_idx], p_cli->idx, '\b');
            //int retval = CliInsertChar(p_cli->input_buffer[p_cli->in_buff_idx], p_cli->idx, 127);

            if (retval)
            {
                CliSendString("\a");
            }
            else
            {
                p_cli->idx--;  // we tested for idx=0 up above
            }
            break;
        default:
            if (p_cli->idx >= LEN_STD_STR - 1)
            {
                p_cli->input_buffer[p_cli->in_buff_idx][LEN_STD_STR - 1] = 0;

                CliSendString("\a");
                return 0;
            }

            switch (esc_state)
            {
                case eNO_ESC_SEQ:
                    if (rec_char == 27) // ESC
                    {
#ifdef DEBUG
				escape_buff[eb_idx++] = rec_char;
#endif
                        esc_state = eESC_RECVD;
                    }
                    else   // normal char
                    {
#ifdef DEBUG
                        eb_idx = 0;
#endif
                        retval = CliInsertChar(p_cli->input_buffer[p_cli->in_buff_idx], p_cli->idx, rec_char);

                        if (!retval)
                        {
                            p_cli->idx++;
                        }
                    }
                    break;
                case eESC_RECVD:
#ifdef DEBUG
			escape_buff[eb_idx++] = rec_char;
#endif

                    if (rec_char == '[')
                    {
                        esc_state = eBRCKT_RECVD;
                    }
                    else if (rec_char == 'O')	// minicom does END with ^[OF
                    {
                        esc_state = eO_RECVD;
                    }
                    else
                    {
                        esc_state = eNO_ESC_SEQ;
                        eb_idx = 0;

                        CliSendString("\a");
                    }
                    break;
                case eO_RECVD:
                case eBRCKT_RECVD:
#ifdef DEBUG
			escape_buff[eb_idx++] = rec_char;
#endif

                    if (rec_char > '0' && rec_char < '9')
                    {
                        esc_number = rec_char - '0';
                        esc_state = eESC_NUM;
                        break;
                    }
                    else if (rec_char == 'a' || rec_char == 'A') /* up */
                    {
                        // stash buffer if it is the current one
                        // decrement in_buff_idx
                        // load historic buffer into current one
                        // clear CLI
                        // print current buffer

                        if (history_buff_idx == p_cli->in_buff_idx)
                        {
                            strcpy(stashed_buffer, p_cli->input_buffer[p_cli->in_buff_idx]);
                        }

                        --history_buff_idx;
                        if (history_buff_idx < 0)
                        {
                            history_buff_idx = NUM_IN_CMD_RECALL - 1;
                        }

                        if (history_buff_idx == p_cli->in_buff_idx || p_cli->input_buffer[history_buff_idx][0] == 0)
                        {
                            ++history_buff_idx;
                            if (history_buff_idx > NUM_IN_CMD_RECALL - 1)
                            {
                                history_buff_idx = 0;
                            }
                            CliSendString("\a");
                        }

                        CliClear();

                        // copy buffer to curr_buff, update idx, restore in_buff_idx
                        strcpy(p_cli->input_buffer[p_cli->in_buff_idx], p_cli->input_buffer[history_buff_idx]);
                        p_cli->idx = strlen(p_cli->input_buffer[history_buff_idx]);

                        CliSendString(p_cli->input_buffer[p_cli->in_buff_idx]);

                    }
                    else if (rec_char == 'b' || rec_char == 'B') /* down */
                    {
                        int prev_history_buff_idx = history_buff_idx;

                        if (history_buff_idx != p_cli->in_buff_idx)
                        {
                            ++history_buff_idx;
                            if (history_buff_idx > NUM_IN_CMD_RECALL - 1)
                            {
                                history_buff_idx = 0;
                            }
                        }
                        else
                        {
                            CliSendString("\a"); // bonk
                            //break;
                        }

                        CliClear();

                        if (history_buff_idx == p_cli->in_buff_idx && prev_history_buff_idx != history_buff_idx)
                        {
                            // cannot go down any further, restore stashed buffer

                            strcpy(p_cli->input_buffer[p_cli->in_buff_idx], stashed_buffer);
                            stashed_buffer[0] = 0;
                        }
                        else
                        {
                            strcpy(p_cli->input_buffer[p_cli->in_buff_idx], p_cli->input_buffer[history_buff_idx]);
                        }

                        p_cli->idx = strlen(p_cli->input_buffer[p_cli->in_buff_idx]);
                        CliSendString(p_cli->input_buffer[p_cli->in_buff_idx]);

                    }
                    else if (rec_char == 'c' || rec_char == 'C') /* right */
                    {
                        /*CliSendString("right");*/

                        if (p_cli->idx < strlen(p_cli->input_buffer[p_cli->in_buff_idx]))
                        {
                            p_cli->idx++;
                            CliSendString("\e[C");
                        }
                        else
                        {
                            CliSendString("\a");
                        }
                    }
                    else if (rec_char == 'd' || rec_char == 'D') /* left */
                    {
                        /*CliSendString("left");*/

                        if (p_cli->idx)
                        {
                            p_cli->idx--;
                            CliSendString("\e[D");
                        }
                        else
                        {
                            CliSendString("\a");
                        }
                    }
                    else if (rec_char == 'h' || rec_char == 'H') /* home */
                    {
                        p_cli->idx = 0;

                        CliUtoa(LEN_PROMPT, str_buffer, 10);

                        CliSendString("\r\e[");
                        CliSendString(str_buffer);
                        CliSendString("C");
                    }
                    else if (rec_char == 'f' || rec_char == 'F') /* end */
                    {
                        p_cli->idx = strlen(p_cli->input_buffer[p_cli->in_buff_idx]);

                        CliUtoa(LEN_PROMPT + p_cli->idx, str_buffer, 10);

                        CliSendString("\r\e[");
                        CliSendString(str_buffer);
                        CliSendString("C");
                    }

                    esc_state = eNO_ESC_SEQ;
                    eb_idx = 0;

                    break;
                case eESC_NUM:
#ifdef DEBUG
			escape_buff[eb_idx++] = rec_char;
#endif

                    if (rec_char != '~')
                    {
                        esc_number = 0;
                        esc_state = eNO_ESC_SEQ;
                        eb_idx = 0;
                        break;
                    }
                    esc_state = eVT_SEQ;	// no diff, fall through
                case eVT_SEQ:
                    switch (esc_number)
                    {
                        case 1:  // home (VT102)
                            p_cli->idx = 0;

                            CliUtoa(LEN_PROMPT, str_buffer, 10);

                            CliSendString("\r\e[");
                            CliSendString(str_buffer);
                            CliSendString("C");

                            break;
                        case 3: // delete (not DEL)
                            retval = CliInsertChar(p_cli->input_buffer[p_cli->in_buff_idx], p_cli->idx, 127);

                            if (retval)
                            {
                                CliSendString("\a");
                            }
                            break;
                        case 4:  // end (VT102)
                            p_cli->idx = strlen(p_cli->input_buffer[p_cli->in_buff_idx]);

                            CliUtoa(LEN_PROMPT + p_cli->idx, str_buffer, 10);

                            CliSendString("\r\e[");
                            CliSendString(str_buffer);
                            CliSendString("C");

                            break;
                        default:
                            break;
                    }

                    esc_number = 0;
                    esc_state = eNO_ESC_SEQ;
                    eb_idx = 0;

                    break;
                default:
                    break;
            }

            break;
    }

    return 0;
}

int CliTxISR()    // ISR for each "ready to send char"
{
    static unsigned char_idx = 0;
    static unsigned buffer_idx = 0;
    static char *buffer = 0;

    if (buffer)
    {
        //if(p_cli->output_buffer[buffer_idx][char_idx])
        if (buffer[char_idx])
        {
            //*p_cli->tx_reg_addr = p_cli->output_buffer[buffer_idx][char_idx];
            *p_cli->tx_reg_addr = buffer[char_idx];
            char_idx++;
        }
        else
        {
            buffer = 0;
            char_idx = 0;
        }
    }
    else
    {
        if (p_cli->output_buffer[buffer_idx] && buffer_idx < NUM_OUT_MSG_QUEUE) // fetch next buffer in queue
        {
            buffer = p_cli->output_buffer[buffer_idx];
            buffer_idx++;
        }
        else
        {
            buffer = 0;
            char_idx = 0;
            buffer_idx = 0;

            memset(p_cli->output_buffer, 0, NUM_OUT_MSG_QUEUE*sizeof(char*));

            p_cli->DisableUartInt();
        }
    }

    return 0;
}

int CliInsertChar(char *str, int position, char character)
{
    int len_str = strlen(str);
    int len_rem = len_str - position;
    char num_rem_chars_str[4] = { 0 };  // up to 999

    switch (character)
    {
        case '\b': // backspace (internal, not from UART)
            if (position == 0)
            {
                return -1;
            }

            if (position < len_str)
            {
                memmove(&str[position - 1], &str[position], len_rem + 1); // copy also \0

                CliSendString("\e[D");
                CliSendString(&str[position - 1]);
                CliSendString(" ");
                //get back the amount of characters printed over:
                // get back len-idx

                CliUtoa(len_rem + 1, num_rem_chars_str, 10);

                CliSendString("\e[");
                CliSendString(num_rem_chars_str);
                CliSendString("D");
            }
            else
            {
                str[position - 1] = 0;
                CliSendString("\b \b");
            }

            break;
        case 127:  // delete (internal, not from UART)

            if (position < len_str)
            {
                memmove(&str[position], &str[position + 1], len_rem); // copy also \0

                CliSendString(&str[position]);
                CliSendString(" ");
                //get back the amount of characters printed over:
                // get back len-idx

                CliUtoa(len_rem, num_rem_chars_str, 10);

                CliSendString("\e[");
                CliSendString(num_rem_chars_str);
                CliSendString("D");
            }
            else
            {
                return -1;
            }

            break;
        default:
            if (len_str + 1 >= LEN_STD_STR - 1)
            {
                return -1;
            }

            if (position < len_str)
            {
                memmove(&str[position + 1], &str[position], len_rem + 1); // copy also \0
                str[position] = character;

                CliSendString(&str[position]);

                // move cursor back the amount of characters printed over:
                CliUtoa(len_rem, num_rem_chars_str, 10);

                CliSendString("\e[");
                CliSendString(num_rem_chars_str);
                CliSendString("D");
            }
            else if (position == len_str)
            {
                str[position] = character;
                str[position + 1] = 0;

                CliSendString(&str[position]);
            }
            break;
    }

    return 0;
}

int CliClear()
{
    CliSendString("\r");
    CliSendString(white_spaces);
    CliSendString(prompt);
}

void CliSendString(const char *orig)
{
    for (unsigned i = 0; i < NUM_OUT_MSG_QUEUE; ++i)
    {
        if (!p_cli->output_buffer[i])
        {
            p_cli->output_buffer[i] = orig;
            break;
        }
    }
    p_cli->EnableUartInt();
}

int CliHandleInput()
{
    static char temp_buffer[LEN_STD_STR] = { 0 };

    if (p_cli->input_buffer[p_cli->in_buff_idx][0])
    {
        strcpy(temp_buffer, p_cli->input_buffer[p_cli->in_buff_idx]);

        char *token = strtok(temp_buffer, " \t");

        for (unsigned i = 0; commands[i].handle[0]; ++i)
        {
            if (!strcmp(token, commands[i].handle))
            {
                token = strtok(NULL, "");
                if(commands[i].callback)
                {
                    CliSendString("\r\n");
                    commands[i].callback(token);
                }
            }
        }

        /* prepare a fresh input buffer */
        if (++p_cli->in_buff_idx > NUM_IN_CMD_RECALL - 1)
        {
            p_cli->in_buff_idx = 0;
        }

        p_cli->input_buffer[p_cli->in_buff_idx][0] = 0;
    }

    /* prepare the CLI */
    CliSendString("\r\n");
    CliSendString(prompt);

    p_cli->idx = 0;
    p_cli->was_input_received = 0;

    return 0;
}

