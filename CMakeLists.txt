cmake_minimum_required(VERSION 3.10)

project(arg-parse C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(LIB_SOURCES src/seargs.c)

add_library(seargs STATIC ${LIB_SOURCES})

target_include_directories(seargs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/headers)

add_executable(example example/eg.c)

target_link_libraries(example PRIVATE seargs)

install(TARGETS seargs DESTINATION lib)
install(DIRECTORY headers/ DESTINATION include)