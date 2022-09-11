@echo off
del *.o *.gch *.exe 2>nul

if '%1%' equ '-c' (
    goto exit
)
windres -i "resource/res.rc" -o "res.o"

gcc *.h *.c *.o -o tool.exe -mwindows -lwininet -lVersion

:exit