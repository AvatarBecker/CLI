# Command Line Interface (CLI) for embedded projects (WIP)

I was tired of writing ad hoc CLIs, so I wrote one to rule them all (all of mine that is).

I tried to make it similar to bash. It has a **commands history**, accessed with <kbd>&#8593;</kbd> and <kbd>&#8595;</kbd>. You can also **navigate and edit the command** using <kbd>&#8592;</kbd>/<kbd>&#8594;</kbd>, <kbd>Home</kbd>/<kbd>End</kbd>, and <kbd>Backspace</kbd>/<kbd>Del</kbd>.

I've tested it with minicom, screen, and PuTTY during development and tried to contemplate their escape sequences for aforementioned keys.

I plan to test it with different MCUs and upload an example project here. For now there is one available for NXP S32K148 using S32DS.

The code per se is in the "cli" folder.
The list of commands is at the end of file "cli_cmds.c". In the same file are defined the callbacks functions for the commands.

It is interrupt oriented (therefore CliSendString is non blocking), relying on the "Rx buffer full" and "Rx buffer empty" (for each character) interrupts of the UART peripheral. For this, just call CliRxISR and CliTxISR where appropriate.

The structure tCli needs to be passed functions for enabling and disabling the Tx interrupts to function pointers EnableUartInt and DisableUartInt, respectively.
It also needs the address of the Rx and Tx char buffers to be assigned to rx_reg_addr and tx_reg_addr, respectively.
