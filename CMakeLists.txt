cmake_minimum_required(VERSION 2.8)
project(pecar)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -DCS_DEBUG=2 -std=c++11 -pthread -O0 -g3 -Wall -ferror-limit=5")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -std=c++11 -pthread -O2 -Wall")

string(TOLOWER "${CMAKE_BUILD_TYPE}" PECAR_BUILD_TYPE)
if (PECAR_BUILD_TYPE STREQUAL "debug")
    message("building type: ${CMAKE_BUILD_TYPE}")
    set(CMAKE_CXX_COMPILER "clang++")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
else()
    message("building type: ${CMAKE_BUILD_TYPE}")
    set(CMAKE_CXX_COMPILER "g++")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic -pthread")

set(SOURCE_FILES src/main.cpp)
add_executable(pecar ${SOURCE_FILES} 
    src/predef.hpp 
    src/main.cpp 
    src/Portal.hpp 
    src/Config.cpp 
    src/Config.hpp  
    src/Aside.cpp  
    src/Aside.hpp  
    src/Channel.cpp 
    src/Channel.hpp 
    src/Authority.hpp 
    src/Authenticater.hpp 
    src/Buffer.hpp 
    src/Crypto.hpp
    src/Outlet.hpp
)

link_directories(/usr/lib/x86_64-linux-gnu)
target_link_libraries(pecar 
    libboost_system.so libboost_filesystem.so libboost_program_options.so libcrypto++.so
)
