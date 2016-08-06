#!/bin/bash

read line < src/version.h
tokens=($line)
version=$(echo "${tokens[2]}" | tr -d $'\r')
newversion=$(echo "$version + 1" | bc)
echo "#define BUILD_NUMBER $newversion" > src/version.h
