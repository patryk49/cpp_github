#!/bin/bash

g++ src/$1.cpp -o bin/$1 -g \
	-std=c++20 -Iinclude -fno-exceptions \
	-Wall -Wextra -Wno-attributes -Wno-dangling-else \
	$2 $3 $4 $5
