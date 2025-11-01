# WebServer tests
add_executable(webserver_tests webserver_tests.cpp) 
target_link_libraries(webserver_tests PRIVATE webserver gtest gtest_main gmock)
add_test(NAME webserver_unit_tests COMMAND webserver_tests)

# ThreadPool tests
add_executable(threadpool_tests thread_pool_tests.cpp) 
target_link_libraries(threadpool_tests PRIVATE webserver gtest gtest_main gmock)
add_test(NAME threadpool_unit_tests COMMAND threadpool_tests)
