cmake_minimum_required(VERSION 3.28)

project(CanSniffer LANGUAGES CXX)

set(J1939_NODES 1)
set(J1939_LISTENER_ONLY_MODE ON)

add_subdirectory(mini_j1939)
add_subdirectory(src)
