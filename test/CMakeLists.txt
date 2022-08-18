# copy test files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# tests
add_executable(test-all test.cpp)
target_link_libraries(test-all PRIVATE iopp)
add_test(iopp ${CMAKE_CURRENT_BINARY_DIR}/test-all)

# examples
add_executable(examples examples.cpp)
target_link_libraries(examples PRIVATE iopp)
