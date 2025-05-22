cmake_minimum_required(VERSION 2.8.3)
project(offline_command_sample)

add_compile_options(-std=c++11)
include_directories(
   ../../../include
)

link_directories(
   ../../../libs/x64
   ./
)


add_library(offline_record_lib SHARED
 asr_offline_record_sample.c
 speech_recognizer.c
 linuxrec.c
)
target_link_libraries(offline_record_lib msc pthread rt dl asound stdc++)


