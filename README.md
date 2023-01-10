# Shell_Script
 I created my own version of Shell in C called smallsh. This program provides a prompt for running commands, Handles blank lines and comments. It provides expansion for the variable $$, and it executes 3 commands Exit, Cd, and Status via code built into the shell. It also executes other commands by creating new processes using a function from the Exec family of functions. Supports input and output redirection as well as running commands in foreground and background processes. Lastly it implements custom handlers for 2 signals, SIGINT and SIGTSTP.

How to compile code

Run the follwoing: gcc -o smallsh smallsh.c

then: ./smallsh
