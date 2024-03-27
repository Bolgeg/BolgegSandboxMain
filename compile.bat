@echo off
g++ assembler.cpp -o assemble.exe -D WINDOWS -O3 -std=c++17
windres --input application/windows/manifest.rc --output application/windows/manifest.res --output-format=coff
g++ main.cpp -o emulate.exe application/windows/manifest.res -mwindows -D WINDOWS -O3 -lSDL2 -std=c++17
