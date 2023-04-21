#!/usr/bin/bash

set -uex

as shell.S -o shell.o
ld shell.o -o shell
rm shell.o

