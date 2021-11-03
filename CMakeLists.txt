# referencia para o cmake
#https://github.com/estiveljr/cplex-example


cmake_minimum_required(VERSION 2.8.4)
project(modelocpp)
include(CMakePrintHelpers)

set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -DIL_STD -Wall")
#set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -std=c++14 -DIL_STD -m64 -Wall -Werror")

set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -0g -gldb")
#set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -Og -ggdb")

set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -flto")

#set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX   "-isystem ")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/build")

set(LINKER_OPTIONS                  -Wl,--no-as-needed)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -save-temps=obj")
endif()

find_package(CPLEX)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp)

add_executable(modelocpp ${SOURCE_FILES})

target_link_libraries(modelocpp PRIVATE ${CPLEX_LIBRARIES})
#target_link_libraries(modelocpp PRIVATE dl)

cmake_print_variables(CPLEX_LIBRARIES)