add_executable (wabbitemu
	main.cpp
	wabbitinstance.cpp)	


find_library(NCURSES_PATH ncurses)



target_link_libraries(wabbitemu ${NCURSES_PATH})
target_link_libraries (wabbitemu crane wabbitutilities wabbithardware wabbitcore wabbitinterface)
