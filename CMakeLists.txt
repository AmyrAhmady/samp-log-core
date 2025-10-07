cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(samp-log-core VERSION 0.4)

if (NOT LOGCORE_VERSION) # allow version override
	set(LOGCORE_VERSION ${PROJECT_VERSION})
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/libs/cmake")

# Set target architecture (default: x86)
if(NOT DEFINED TARGET_ARCH)
	set(TARGET_ARCH "x86")
endif()

set(CMAKE_CXX_STANDARD 11) #required for fmt library

if (MSVC)
	add_link_options(/INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
	# link runtime statically
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(UNIX)
	# Apply -m32 flags for x86 architecture
	if(TARGET_ARCH STREQUAL "x86")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		add_link_options(-m32)
	endif()

	# hide non-exported symbols
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

	# link runtime statically
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	add_link_options(
		"-static-libgcc"
		"-static-libstdc++"
	)
endif()

set(LOGCORE_LIBS_DIR "${PROJECT_SOURCE_DIR}/libs")

add_subdirectory(libs/fmt)
mark_as_advanced(FMT_TEST FMT_INSTALL FMT_PEDANTIC FMT_DOC FMT_USE_CPP11 FMT_WERROR)

set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_MSVC_SHARED_RT OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/yaml-cpp)

add_subdirectory(src)
