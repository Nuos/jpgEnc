cmake_minimum_required (VERSION 2.8.9)

set(PROJECT_NAME jpgEnc)
set(PROJECT_LIB jpgEncLib)
set(PROJECT_TEST jpgEncTest)
project(${PROJECT_NAME} CXX)

# unicode build
add_definitions(-DUNICODE -D_UNICODE)

# boost
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package( Boost 1.54 COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

# Visual studio stuff
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

add_subdirectory(src)

# copy test files to binary dir
file(COPY res DESTINATION ${CMAKE_BINARY_DIR})