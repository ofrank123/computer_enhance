nasm.exe input%1.asm
.\ex01.exe input%1 > test%1.asm
nasm.exe test%1.asm
fc.exe .\input%1 .\test%1

