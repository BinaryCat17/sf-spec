cmake_minimum_required(VERSION 3.25)
project(sf-spec LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Modules
add_subdirectory(base)
add_subdirectory(isa)
