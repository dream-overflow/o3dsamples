cmake_minimum_required (VERSION 2.8)
project (O3DSAMPLES)

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

include(dreamoverflow)

set (CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} $ENV{PREFIX})
# set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_ROOT}/Modules)
# message(${CMAKE_MODULE_PATH})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_PREFIX_PATH $ENV{PREFIX} ${CMAKE_PREFIX_PATH})
	set(CMAKE_INCLUDE_PATH $ENV{PREFIX}/include ${CMAKE_INCLUDE_PATH})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(!${O3D_TARGET_ANDROID})
    find_package(OpenGL REQUIRED)
endif()

find_package(OpenAL REQUIRED)
find_package(Objective3D REQUIRED)
#find_package(Bullet REQUIRED)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(X11 REQUIRED)
	find_package(Xrandr REQUIRED)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_package(X11 REQUIRED)
	find_package(Xrandr REQUIRED)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#	set(O3D_INCLUDES "E:/dev/dreamoverflow/o3d/include")
#	set(O3D_LIBRARIES_DIR "E:/dev/dreamoverflow/o3d/build-x86_64/lib")
#	set(O3D_LIBRARIES_DIR "E:/dev/dreamoverflow/o3d/build-x86_64-dbg/lib")
#	include_directories(${O3D_INCLUDES})
#	link_directories(${O3D_LIBRARIES_DIR})
#	link_directories("E:/dev/dreamoverflow/deps/lib")
#	link_directories("E:/dev/dreamoverflow/deps/lib")
#	link_directories("E:/libs/OpenAL/libs/Win32")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(LIBRARY_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/bin)

include_directories("${PROJECT_SOURCE_DIR}/include")

#----------------------------------------------------------
# options
#----------------------------------------------------------

IF(${O3D_USE_SSE2})
	message("-- SIMD/SSE2 support enabled")
ENDIF(${O3D_USE_SSE2})

include_directories(${OBJECTIVE3D_INCLUDE_DIR})
include_directories(${OBJECTIVE3D_INCLUDE_DIR_objective3dconfig})

link_directories(${OBJECTIVE3D_LIBRARY_DIR})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
    # set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    if (O3D_USE_SSE2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
    endif (O3D_USE_SSE2)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Android")

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

# android only
if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(EXTRA_CXX o3d/third/android/android_native_app_glue.c)  # @todo improve
    set(ANDROID_LIBRARY "-llog -landroid -ldl -lEGL -lGLESv3")
else()
    set(EXTRA_CXX "")
endif()

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

file(COPY ${OBJECTIVE3D_SHADERS_ZIP} DESTINATION ${CMAKE_BINARY_DIR})

#----------------------------------------------------------
# targets
#----------------------------------------------------------

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    add_library(minimal SHARED minimal/minimal.cpp android/android_native_app_glue.c)
    add_library(window SHARED window/window.cpp android/android_native_app_glue.c)
    add_library(audio SHARED audio/audio.cpp android/android_native_app_glue.c)
    add_library(ms3d SHARED ms3d/ms3d.cpp android/android_native_app_glue.c)
    add_library(pclodterrain SHARED pclodterrain/pclodterrain.cpp android/android_native_app_glue.c)
    add_library(heightmap SHARED heightmap/heightmap.cpp android/android_native_app_glue.c)
    add_library(primitives SHARED primitives/primitives.cpp android/android_native_app_glue.c)
    add_library(gui SHARED gui/gui.cpp android/android_native_app_glue.c)

    set(LINKER_EXTRA "${ANDROID_LIBRARY} -u ANativeActivity_onCreate")
else()
    add_executable(minimal minimal/minimal.cpp)
    add_executable(window window/window.cpp)
    add_executable(audio audio/audio.cpp)
    add_executable(ms3d ms3d/ms3d.cpp)
    add_executable(pclodterrain pclodterrain/pclodterrain.cpp)
    add_executable(heightmap heightmap/heightmap.cpp)
    add_executable(primitives primitives/primitives.cpp)
    add_executable(gui gui/gui.cpp)

    set(LINKER_EXTRA "")  # ${OPENGL_gl_LIBRARY}
endif()

add_library(dynlib SHARED minimal/dynlib.cpp)

target_link_libraries(minimal ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(window ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(audio ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(ms3d ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(pclodterrain ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(heightmap ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(primitives ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
target_link_libraries(gui ${OBJECTIVE3D_LIBRARY} ${LINKER_EXTRA})
