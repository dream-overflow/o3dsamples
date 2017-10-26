cmake_minimum_required (VERSION 2.6)
project (O3D_EXAMPLES)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   message(STATUS "Setting build type to 'Debug' as none was specified.")
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
   # Set the possible values of build type for cmake-gui
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
     "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT MINGW AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /D_DEBUG /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /DNDEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER /DO3D_DISABLE_GPU_MEMORY_MANAGER /DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUG          "/D_DEBUG /Zi /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUGFULL      "/D_DEBUG /Zi /EHsc /MP /MDd")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/O2 /Zi /D_DEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_RELEASE          "/O2 /DNDEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER /DO3D_DISABLE_GPU_MEMORY_MANAGER /DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUG            "/D_DEBUG /Zi /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUGFULL        "/D_DEBUG /Zi /EHsc /MP /MDd")
else()
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_CXX_FLAGS_DEBUGFULL      "-O0 -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUG            "-O0 -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
	set(CMAKE_C_FLAGS_DEBUGFULL        "-O0 -ggdb -D_DEBUG")
endif()

#----------------------------------------------------------
# externals libraries
#----------------------------------------------------------

include(FindOpenGL)
include(FindOpenAL)

#include(Bullet)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	include(FindX11)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include(FindX11)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		#	set(O3D_INCLUDES "E:/dev/o3dgit/o3d/api/include")
		#	set(O3D_LIBRARIES_DIR "E:/dev/o3dgit/o3d/api/dist/lib/Release")
		#	include_directories(${O3D_INCLUDES})
		#	link_directories(${O3D_LIBRARIES_DIR})
		#	link_directories("E:/dev/o3d/trunk/external/lib")
#	link_directories("E:/dev\o3dgit/o3d/deps/lib")
#	link_directories("E:/libs/OpenAL/libs/Win32")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(LIBRARY_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/bin)

include_directories("${PROJECT_SOURCE_DIR}/include")

#----------------------------------------------------------
# options
#----------------------------------------------------------

option(O3D_USE_SSE2 "Use SSE2 optimizations" ON)

# objective3d Config.h
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(MINGW)
		set(LIB_PATH $ENV{CROSSROAD_PREFIX})
	else()
		set(LIB_PATH $ENV{LIB_PATH})
	endif()
else()
	set(LIB_PATH $ENV{PREFIX})
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	include_directories(${LIB_PATH}/lib/objective3d-dbg)
	set(LIB_EXT "-dbg")
elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
	include_directories(${LIB_PATH}/lib/objective3d-odbg)
	set(LIB_EXT "-odbg")
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
	include_directories(${LIB_PATH}/lib/objective3d)
	set(LIB_EXT "")
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
	if (O3D_USE_SSE2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	endif (O3D_USE_SSE2)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
	if (O3D_USE_SSE2)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	endif (O3D_USE_SSE2)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	option(O3D_USE_SSE2 "Compile SSE2 optimizations" OFF)
	if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -std=c++14 -fexceptions -Wextra -DO3D_WIN32")
	endif()
    if(O3D_USE_SSE2)
		if(MINGW)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
		endif()
    endif (O3D_USE_SSE2)

	# Windows subsystem
	if(MSVC)
		set(CMAKE_CREATE_WIN32_EXE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
	endif(MSVC)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (O3D_USE_SSE2)
	set(O3D_USE_SSE2 "")
endif (O3D_USE_SSE2)

#----------------------------------------------------------
# sub-directories
#----------------------------------------------------------

#add_subdirectory(minimal)
#add_subdirectory(window)
#add_subdirectory(pclodterrain)
#add_subdirectory(audio)
#add_subdirectory(ms3d)
#add_subdirectory(heightmap)
#add_subdirectory(primitives)
#add_subdirectory(gui)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#----------------------------------------------------------
# resources
#----------------------------------------------------------

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	file(COPY ${LIB_PATH}/share/o3d/shaders.zip DESTINATION ${CMAKE_BINARY_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	file(COPY ${LIB_PATH}/share/o3d/shaders.zip DESTINATION ${CMAKE_BINARY_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	file(COPY ${LIB_PATH}/share/o3d/shaders.zip DESTINATION ${CMAKE_BINARY_DIR})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#----------------------------------------------------------
# targets
#----------------------------------------------------------

add_executable(minimal minimal/minimal.cpp)
target_link_libraries(minimal objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_library(dynlib SHARED minimal/dynlib.cpp)

add_executable(window window/window.cpp)
target_link_libraries(window objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_executable(audio audio/audio.cpp)
target_link_libraries(audio objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_executable(ms3d ms3d/ms3d.cpp)
target_link_libraries(ms3d objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_executable(pclodterrain pclodterrain/pclodterrain.cpp)
target_link_libraries(pclodterrain objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_executable(heightmap heightmap/heightmap.cpp)
target_link_libraries(heightmap objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_executable(primitives primitives/primitives.cpp)
target_link_libraries(primitives objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})

add_executable(gui gui/gui.cpp)
target_link_libraries(gui objective3d${LIB_EXT} ${OPENGL_gl_LIBRARY})
