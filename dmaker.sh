#!/bin/bash

g++ src/$1.cpp -o bin/$1 -g \
	-std=c++20 -Iinclude -fno-exceptions \
	-Wall -Wextra -Wno-attributes -Wno-dangling-else
