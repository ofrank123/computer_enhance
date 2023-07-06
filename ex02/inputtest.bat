@echo off
nasm.exe input%1.asm
@echo on
.\ex02.exe input%1
@echo off
.\ex02.exe input%1 > test%1.asm
nasm.exe test%1.asm
fc.exe /b .\input%1 .\test%1

