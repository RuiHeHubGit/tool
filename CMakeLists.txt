cmake_minimum_required(VERSION 3.23)
project(tool C)

set(CMAKE_C_STANDARD 99)

include_directories(C:\\OpenSSL-Win64\\include)
link_directories(C:\\OpenSSL-Win64\\lib)

link_libraries(ws2_32)
link_libraries(openssl libssl libcrypto)

add_executable(tool openProjects.h openProjects.c main.c winFrame.c winFrame.h md5.c md5.h uuid.c uuid.h unicode.c unicode.h jsonFormat.c jsonFormat.h levenshtein.c levenshtein.h download.h download.c)
