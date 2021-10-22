/*
 * cli_cmds.c
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
// include any hardware support header you need here...

int Help(tCli *p_cli, char *args)
{
    /* Print cmd descriptions */
    //CliSendString(p_cli, "\n\rHelp yourself! (WIP, will print descriptions)");

    for (unsigned i = 0; commands[i].handle[0]; ++i)
    {
        if (commands[i].handle)
        {
            CliSendString(p_cli, commands[i].handle);
            CliSendString(p_cli, " - ");

            if (commands[i].description)
            {
                CliSendString(p_cli, commands[i].description);
                CliSendString(p_cli, "\r\n");
            }
        }
    }

    return 0;
}

int SayHello(tCli *p_cli, char *args)
{
    CliSendString(p_cli, "Hello World!");
    
    return 0;
}

int ReadAddr(tCli *p_cli, char *args)
{
    unsigned long *addr = 0;
    unsigned long value = 0;
    char *arg_end = NULL;
    char addr_str[33] = { 0 };
    char value_str[33] = { 0 };
    
    addr = strtoul(args, &arg_end, 0);
    *arg_end = 0; /*null terminate after first arg*/
    
    CliUtoa(addr, addr_str, 16);
    
    CliSendString(p_cli, "read 0x");
    CliSendString(p_cli, addr_str);
    CliSendString(p_cli, ": ");
    
    if (addr)
    {
        value = *addr;
        
        CliUtoa(value, value_str, 16);
        
        CliSendString(p_cli, "0x");
        CliSendString(p_cli, value_str);
    }
    
    return 0;
}

int WriteAddr(tCli *p_cli, char *args)
{
    CliSendString(p_cli, "write: ");
    CliSendString(p_cli, args);
    
    return 0;
}


/* @formatter:off */

tCmd commands[] =
{
        {
            "help",
            "Prints commands descriptions.",
            Help
        },
        {
            "hello",
            "Prints a greeting.",
            SayHello
        },
        {
            "read",
            "read <addr 0xh/d>",
            ReadAddr
        },
        {
            "write",
            "read <addr 0xh/d> <value 0xh/d>",
            WriteAddr
        },
        {
            "null_test",
            "Just a test of NULL callback.",
            0
        },
        {
            "",
            "",
            (void*)0
        }
};

