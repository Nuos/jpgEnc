# Test project files
set(SOURCE_FILES_TEST
    "ImageTest.cpp"
    )

add_executable(${PROJECT_TEST} ${SOURCE_FILES_TEST}) 
target_link_libraries(${PROJECT_TEST} ${Boost_LIBRARIES} ${PROJECT_LIB})

add_custom_target(RUN_TESTS ALL ${PROJECT_TEST} 
                  DEPENDS ${PROJECT_TEST}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  )