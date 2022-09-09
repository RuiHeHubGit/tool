del *.o
windres -i "resource/icon.rc" -o "icon.o"
gcc -c *.h *.c -mwindows
gcc -g *.o -o tool.exe -mwindows