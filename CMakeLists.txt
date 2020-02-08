cmake_minimum_required(VERSION 3.5)
project(OJCLIENT)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS -static)
set(CMAKE_EXE_LINKER_FLAGS -L/usr/lib/x86_64-linux-gnu/)  #work for cpprest

#LINK_DIRECTORIES("/usr/lib/x86_64-linux-gnu/")

add_executable(DAEMON daemon_app.cpp
        Rest/Client.h Rest/Client.cpp Rest/Rest.h Rest/Rest.cpp  Rest/HustojClient.cpp Rest/HustojClient.h
        Common/Utils.h Common/Utils.cpp Common/Exceptions.h Common/Exceptions.cpp Common/Logger.cpp Common/Logger.h
        Core/Daemon.cpp Core/Daemon.h)

add_executable(JUDGE judge_app.cpp
        Rest/Client.h Rest/Client.cpp Rest/Rest.h Rest/Rest.cpp  Rest/HustojClient.cpp Rest/HustojClient.h
        Common/Utils.h Common/Utils.cpp Common/Exceptions.h Common/Exceptions.cpp Common/Logger.cpp Common/Logger.h
        Core/Daemon.cpp Core/Daemon.h Core/Judge.h Core/Judge.cpp Core/SafeSysCall.h)

add_executable(TESTREST Test/test_rest.cpp
        Rest/Rest.cpp Rest/Rest.h Rest/Client.cpp Rest/Client.h Rest/HustojClient.cpp Rest/HustojClient.h
        Common/Utils.h Common/Utils.cpp Common/Exceptions.h Common/Exceptions.cpp Common/Logger.cpp Common/Logger.h)


target_link_libraries(TESTREST libcpprest.so boost_filesystem boost_system ssl crypto)

target_link_libraries(DAEMON libcpprest.so boost_filesystem boost_system ssl crypto)

target_link_libraries(JUDGE libcpprest.so boost_filesystem boost_system ssl crypto)

