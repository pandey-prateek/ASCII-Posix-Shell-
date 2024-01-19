# POSIX SHELL

We have developed a POSIX compatible shell which supports a subset of features of the default linux shell. Our shell creates its own .myrc file to store its environment variables. It supports various features which are mentioned below.

## Execution

**complie**: g++ main.cpp
**run**: ./a.out

## Commands Supported

1.  IO Redirection
    `>>` and `>`
2.  Pipe
    `|`
3.  Root user prompt change
    `sudo -i`
    For exiting, use `exit`
4.  Background command execution
    `&` is passed as last token of command
5.  Make process foreground
    `fg`
6.  Environment variables supported
    - PATH
    - HOME
    - USER
    - HOSTNAME
    - PS1
7.  `~` is associated with HOME
8.  Alias support for commands
    `alias ll='ls -l'`
9.  `$$` : shows pid of current process
10. `$?`: shows status of last executed process
11. `record start`: starts recording input and output of shell in "recording.txt" file
12. `record stop`: stops recording
13. `history` : shows history of commands upto size stored in HISTSIZE
14. `history keyword`: filters and shows all lines in history containing keyword
15. `TAB`: auto-completes / shows list of commands starting with the typed prefix
16. `export`
17. `alarm`
18. `open file`: opens a file using the application specified in .myrc
19. `cd path`: changes the directory to given path
20. `pwd`: prints current working directory
21. `env` and `printenv`: displays environment variables stored in .myrc
22. `echo`: to print in shell
23. `exit`:terminate shell

## Assumptions

- HSTSIZE is initialised as 2000
- TAB key is pressed only after typing some prefix of command
- Control keys such as arrows, Ctrl + C etc. are not functional
