cmake_minimum_required (VERSION 2.8.9)

project(jpgEnc CXX)

# unicode build
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

if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Zi /GL /Oi /Ot /Ox /fp:fast")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO")
  
  # activate SSE2 for 32bit builds
  if(NOT ${CMAKE_GENERATOR} MATCHES "Win64")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
  else()
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /favor:INTEL64")
  endif()

endif()

file(GLOB INCLUDE_FILES_JPG_ENC "include/*.h" "include/*.hpp")
file(GLOB SOURCE_FILES_JPG_ENC "src/*.cpp")

add_executable(jpgEnc ${INCLUDE_FILES_JPG_ENC} ${SOURCE_FILES_JPG_ENC}) 
target_link_libraries( jpgEnc ${Boost_LIBRARIES} )