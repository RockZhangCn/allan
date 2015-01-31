project(choudan)

cmake_minimum_required(VERSION 2.6)

if(NOT CMAKE_BUILD_RELEASE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

#Warning Options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
set(CXX_FLAGS
    -g
    #preprocessor
    -DCHECK_PTHREAD_RETURN_VALUE #Macro for preprocessor
    -D_FILE_OFFSET_BITS=64
    #warning
    -Wall
    -Werror
    -Wno-unused-parameter
    -Wold-style-cast
    -Wpointer-arith #disable arithmetic on a "void *"
    -Wshadow #variable shadow
    # link option
    -rdynamic
)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(
    ${choudan_SOURCE_DIR}/include
    ${choudan_SOURCE_DIR}/src)

link_directories(${choudan_BINARY_DIR}/lib)

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " "
    ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(src/base)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(examples)
endif()
