# Main library project files
file(GLOB INCLUDE_FILES_JPG_ENC
          "${CMAKE_SOURCE_DIR}/include/*.h"
          "${CMAKE_SOURCE_DIR}/include/*.hpp"
     )
set(SOURCE_FILES_JPG_ENC
    Image.cpp
    Bitstream.cpp
    )

add_library(${PROJECT_LIB} ${INCLUDE_FILES_JPG_ENC} ${SOURCE_FILES_JPG_ENC}) 

# Main executable project files
add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB})

add_subdirectory(test)