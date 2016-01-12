#!/bin/bash 
./parser $1
aarch64-linux-gnu-gcc -O0 -static main.S
