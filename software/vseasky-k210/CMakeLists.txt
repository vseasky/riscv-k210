# DO NOT MODIFY THIS FILE, IT WILL BE OVERRIDE!!!

# set this will supress some warnings
set(BUILDING_SDK "yes" CACHE INTERNAL "")

# basic config
if (NOT PROJ)
    get_filename_component(PROJ ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)
    get_filename_component(PROJ ${PROJ} NAME)
    string(REPLACE " " "_" PROJ ${PROJ})
    message(STATUS "PROJ not set, use ${PROJ} as PROJ. Also, you can set it manually. e.g. -DPROJ=hello_world")
else()
    message("PROJ = ${PROJ}")
endif ()
cmake_minimum_required(VERSION 3.0)
include(./cmake/common.cmake)
project(${PROJ} C CXX ASM)

# config self use headers
include(./cmake/macros.internal.cmake)
header_directories(${SDK_ROOT}/lib)
# header_directories(src/${PROJ})
# header_directories(kendryte-standalone-demo/${PROJ})
header_directories(../projects/${PROJ}/src)
# 添加板级支持包的头文件
header_directories(../projects/board)
header_directories(../projects/board/camera)
header_directories(../projects/board/flash)
header_directories(../projects/board/lcd)
header_directories(../projects/board/region_layer)

add_subdirectory(third_party/gsl-lite)
add_subdirectory(third_party/mpark-variant)
add_subdirectory(third_party/nlohmann_json)
# build library first
add_subdirectory(lib)

# 添加板级支持包的源文件
add_source_files(   ../projects/board/*.c 
                    ../projects/board/*.cpp 
                    ../projects/board/*/*.c 
                    ../projects/board/*/*.cpp)
# 应用文件
add_source_files(   ../projects/${PROJ}/src/*.c 
                    ../projects/${PROJ}/src/*.s 
                    ../projects/${PROJ}/src/*.S 
                    ../projects/${PROJ}/src/*.cpp)
# add_source_files(kendryte-standalone-demo/${PROJ}/*.c kendryte-standalone-demo/${PROJ}/*.s kendryte-standalone-demo/${PROJ}/*.S kendryte-standalone-demo/${PROJ}/*.cpp)
include(./cmake/executable.cmake)

