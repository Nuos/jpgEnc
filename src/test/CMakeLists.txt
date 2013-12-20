# Test project files
file(GLOB INCLUDE_FILES_UNIT_TEST
          "${CMAKE_SOURCE_DIR}/include/test/*.h"
          "${CMAKE_SOURCE_DIR}/include/test/*.hpp"
     )
     
set(SOURCE_FILES_UNIT_TEST
    common.cpp
    ImageTest.cpp
    BitstreamGenericTest.cpp
    HuffmanTest.cpp
    PackageMergeTest.cpp
    DctTest.cpp
    CodingTest.cpp
    )
    
set(SOURCE_FILES_PERF_TEST
    perfTest.cpp
    )
    

# Unit tests
add_executable(${PROJECT_TEST} ${SOURCE_FILES_UNIT_TEST} ${INCLUDE_FILES_UNIT_TEST}) 
target_link_libraries(${PROJECT_TEST} ${Boost_LIBRARIES} ${PROJECT_LIB})

add_custom_target(RUN_TESTS ${PROJECT_TEST} 
                  DEPENDS ${PROJECT_TEST}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  )

# Perf tests          
set(PROJECT_TEST_PERF "${PROJECT_TEST}_PERF")
add_executable(${PROJECT_TEST_PERF} ${SOURCE_FILES_PERF_TEST}) 
target_link_libraries(${PROJECT_TEST_PERF} ${Boost_LIBRARIES} ${PROJECT_LIB})

add_custom_target(RUN_TESTS_PERF ${PROJECT_TEST_PERF} 
                  DEPENDS ${PROJECT_TEST_PERF}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  )
                  
# copy test files to binary dir
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})