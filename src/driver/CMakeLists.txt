add_library(driver STATIC options.h options.cpp)
target_include_directories(driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(driver PUBLIC cxx_std_23)
add_executable(zcc main.cpp )
target_link_libraries(zcc PRIVATE driver)
