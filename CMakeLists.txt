cmake_minimum_required (VERSION 2.8.9)

project(jpgEnc CXX)

if (MSVC)
	add_definitions(/MP)
endif(MSVC)

add_definitions(-DUNICODE -D_UNICODE)

# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin) # executables
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin) # shared libs (dll)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # static libs

# This is for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/bin )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# CMAKE_BINARY_DIR: The path to the top level of the build tree.
# CMAKE_SOURCE_DIR: The path to the top level of the source tree.

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package( Boost 1.54 ) #COMPONENTS date_time)
include_directories(${Boost_INCLUDE_DIRS})

#add_subdirectory(src)

file(GLOB INCLUDE_FILES_JPG_ENC "include/*.h" "include/*.hpp")
file(GLOB SOURCE_FILES_JPG_ENC "src/*.cpp")

add_executable(jpgEnc ${INCLUDE_FILES_JPG_ENC} ${SOURCE_FILES_JPG_ENC}) 
target_link_libraries( jpgEnc ${Boost_LIBRARIES} )