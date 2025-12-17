# copy test files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION ${CMAKE_BINARY_DIR})

# examples
add_executable(examples examples.cpp)
target_link_libraries(examples PRIVATE iopp)

# tests
add_executable(test-all test.cpp)
target_link_libraries(test-all PRIVATE iopp)
add_test(iopp ${CMAKE_CURRENT_BINARY_DIR}/test-all)

# tests -- enforce std::istream
add_executable(test-all-istream test.cpp)
target_link_libraries(test-all-istream PRIVATE iopp)
target_compile_definitions(test-all-istream PUBLIC _IOPP_DISABLE_DIRECT_API)
add_test(iopp-istream ${CMAKE_CURRENT_BINARY_DIR}/test-all-istream)
