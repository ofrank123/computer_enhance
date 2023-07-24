@nasm.exe input%1.asm
.\sim8086.exe input%1
@.\sim8086.exe input%1 > test%1.asm
@nasm.exe test%1.asm
@fc.exe /b .\input%1 .\test%1
