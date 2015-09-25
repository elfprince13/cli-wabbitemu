add_library(wabbitutilities 
	exportvar.cpp 
	fileutilities.cpp 
	gif.cpp 
	gifhandle.cpp 
	label.cpp 
	savestate.cpp 
	sendfile.cpp 
	#sound.cpp 
	var.cpp 
	zlibcmp.cpp)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories(wabbitutilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(wabbitutilities LINK_PUBLIC crane)
