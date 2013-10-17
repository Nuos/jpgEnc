# Test project files
set(SOURCE_FILES_TEST
    common.cpp
    ImageTest.cpp
    BitstreamTest.cpp
    BitstreamLongTest.cpp
    )

add_executable(${PROJECT_TEST} ${SOURCE_FILES_TEST}) 
target_link_libraries(${PROJECT_TEST} ${Boost_LIBRARIES} ${PROJECT_LIB})

add_custom_target(RUN_TESTS ${PROJECT_TEST} 
                  DEPENDS ${PROJECT_TEST}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  )
                  
# copy test files to binary dir
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})