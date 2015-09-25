add_executable (wabbitemu
	main.cpp
	wabbitinstance.cpp)	


find_library(NCURSES_PATH ncurses)
find_library(PANEL_PATH panel)
find_library(MENU_PATH menu)
find_library(FORM_PATH form)


target_link_libraries(wabbitemu ${NCURSES_PATH} ${PANEL_PATH} ${MENU_PATH} ${FORM_PATH})
target_link_libraries (wabbitemu crane wabbitutilities wabbithardware wabbitcore wabbitinterface)
