find_package(GTest CONFIG REQUIRED)
add_executable(hello_test hello_test.cpp)
target_link_libraries(hello_test PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(hello_test)