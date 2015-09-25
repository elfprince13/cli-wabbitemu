add_executable (wabbitemu
	main.cpp
	wabbitinstance.cpp)	

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_link_libraries (wabbitemu LINK_PUBLIC crane wabbitutilities wabbithardware wabbitcore wabbitinterface)
