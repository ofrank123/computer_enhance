@echo OFF
call .\build.bat
echo -- DECODE --
nasm.exe input%1.asm
.\sim8086.exe input%1
echo -- EXECUTION --
.\sim8086.exe --exec input%1
