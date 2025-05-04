cmake_minimum_required(VERSION 3.20.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CCache)

project(LabOSIS VERSION 1.0.0 LANGUAGES C)

set(SRC_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB SRC_SOURCES ${SRC_SOURCES_DIR}/*.c)

add_executable(lab2 ${SRC_SOURCES})

target_compile_options(lab2 PRIVATE -Wall -Wextra -Wpedantic)

target_compile_definitions(lab2 PRIVATE $<$<CONFIG:Debug>:DEBUG>)
