#!/bin/bash
g++ assembler.cpp -o assemble -O3 -pthread -std=c++17
g++ main.cpp -o emulate -O3 -lSDL2 -pthread -Wl,-rpath . -std=c++17