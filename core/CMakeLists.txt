add_library (wabbitcore
	alu.cpp
	control.cpp
	core.cpp
	device.cpp
	indexcb.cpp)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (wabbitcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
