@nasm.exe input%1.asm
.\ex03.exe input%1
@.\ex03.exe input%1 > test%1.asm
@nasm.exe test%1.asm
@fc.exe /b .\input%1 .\test%1
