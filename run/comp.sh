#!/bin/bash 

#./parser $1
aarch64-linux-gnu-gcc -O0 -static main.S -o $1.out

cp $1.out ../hw5/vm_share
