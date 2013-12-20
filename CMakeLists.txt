cmake_minimum_required (VERSION 2.8.11.2)

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

if(${CMAKE_SIZEOF_VOID_P} MATCHES "8")
message("Building for x64...")
set(BOOST_LIBRARYDIR "$ENV{BOOST_ROOT}/lib/x64/")
message("Searching boost x64 libs in '$ENV{BOOST_ROOT}/lib/x64/'")
endif()

find_package( Boost 1.54 COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

# Visual studio stuff
if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  message("Setting Visual C++ Flags")
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /openmp")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Zi /GL /O2 /fp:fast /openmp")
  
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG /INCREMENTAL")
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /LTCG")
  SET(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
  
  # activate SSE2 for 32bit builds
  if(NOT ${CMAKE_GENERATOR} MATCHES "Win64")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
  else()
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /favor:INTEL64")
  endif()

endif()

add_subdirectory(src)