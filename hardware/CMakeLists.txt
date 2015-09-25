add_library(wabbithardware
	81hw.cpp
	83hw.cpp
	83phw.cpp
	83psehw.cpp
	86hw.cpp
	keys.cpp
	lcd.cpp
	link.cpp)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories(wabbithardware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
