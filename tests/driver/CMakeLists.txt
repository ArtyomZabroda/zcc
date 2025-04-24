add_executable(driver_test options_test.cpp)
target_link_libraries(driver_test PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(driver_test PRIVATE driver)
gtest_discover_tests(driver_test)