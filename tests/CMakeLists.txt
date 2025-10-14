# Add test executable
add_executable(webserver_tests webserver_tests.cpp) 
target_link_libraries(webserver_tests PRIVATE webserver gtest gtest_main gmock)

# Register test
add_test(NAME unit_tests COMMAND webserver_tests)
