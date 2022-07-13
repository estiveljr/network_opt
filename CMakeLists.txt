# referencia para o cmake
#https://github.com/estiveljr/cplex-example


cmake_minimum_required(VERSION 2.8.4)
project(modelocpp)
include(CMakePrintHelpers)

set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# DEFINE MANUALMENTE SE O AMBIENTE É WSL
set(WSL 1)
if(WSL)
    set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -DIL_STD -Wall ")
#    set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -L/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux")
#    set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -L/opt/ibm/ILOG/CPLEX_Studio201/concert/lib/x86-64_linux")
#    set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -lilocplex -lconcert -lcplex -lm -lpthread")
    set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -std=c++17 -LD_DEBUG=all")


    set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -O0 -ggdb3 -gdwarf-3")
else()
    set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -std=c++17 -DIL_STD -Wall")

    set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -O0")
    #set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -Og -ggdb3")
endif()
cmake_print_variables(WSL)
cmake_print_variables(CMAKE_CXX_FLAGS)
#set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -std=c++17 -DIL_STD -m64 -Wall -Werror")





set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -flto")

#set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX   "-isystem ")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/build")

set(LINKER_OPTIONS                  -Wl,--no-as-needed)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -save-temps=obj")
endif()

#para o WSL
#set(CMAKE_CXX_COMPILER "")

find_package(CPLEX)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp)

add_executable(modelocpp ${SOURCE_FILES})

target_link_libraries(modelocpp PRIVATE ${CPLEX_LIBRARIES})
#target_link_libraries(modelocpp PRIVATE dl)

cmake_print_variables(CPLEX_LIBRARIES)