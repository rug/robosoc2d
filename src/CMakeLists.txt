cmake_minimum_required(VERSION 3.10)

project(robosoc2d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS True)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_R2S_DEBUG") #creates the "#define _R2S_DEBUG" Macro

#file(GLOB robosoc2d_SRC "*.cpp" )	# put all .cpp files names in the variable robosoc2d_SRC
set(robosoc2dc_SRC "simulator.cpp" "simple_player.cpp" "main.cpp")
set(robosoc2dgui_SRC "simulator.cpp" "simple_player.cpp" "r2gui.cpp" "main_gui.cpp")

add_executable(robosoc2dc ${robosoc2dc_SRC})

target_include_directories(robosoc2dc PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )

if (WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	set(wxWidgets_CONFIGURATION mswu)
	set(wxWidgets_ROOT_DIR  $ENV{WXWIN}) # "c:/wx/"
	if(CMAKE_CL_64)
    	set(wxWidgets_LIB_DIR  "$ENV{WXWIN}/lib/vc14x_x64_dll") #"c:/wx/lib/vc14x_x64_dll"
	else()
    	set(wxWidgets_LIB_DIR  "$ENV{WXWIN}/lib/vc14x_dll") #"c:/wx/lib/vc14x_dll"
	endif()
endif ()

find_package(wxWidgets COMPONENTS core base)

if(wxWidgets_FOUND)
	if (UNIX)
		add_executable(robosoc2dgui ${robosoc2dgui_SRC})
	elseif (WIN32)
		add_executable(robosoc2dgui WIN32 ${robosoc2dgui_SRC})
	endif ()

	include(${wxWidgets_USE_FILE})

	target_include_directories(robosoc2dgui PUBLIC
							"${PROJECT_SOURCE_DIR}"
							)

	target_link_libraries(robosoc2dgui ${wxWidgets_LIBRARIES})
else()
	message(WARNING "please install wxWidgets to compile the GUI launcher")
endif()


if (UNIX)
    target_link_libraries(robosoc2dc m)	# to link the c math library (libm)
	if(wxWidgets_FOUND)
    	target_link_libraries(robosoc2dgui m)	# to link the c math library (libm)
	endif()
endif ()



