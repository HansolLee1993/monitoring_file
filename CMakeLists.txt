cmake_minimum_required(VERSION 3.27)
project(monitoring_file C)

set(CMAKE_C_STANDARD 11)

add_executable(monitoring_file main.c
        src/conversion.c)
