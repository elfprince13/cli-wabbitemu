cmake_minimum_required (VERSION 2.8.11)
project (CLI-WabbitEmu C CXX)

add_subdirectory(libs/libcrane)
add_subdirectory(utilities)
add_subdirectory(hardware)
add_subdirectory(core)
add_subdirectory(interface)
add_subdirectory(cli)
