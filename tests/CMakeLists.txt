add_executable(raytracer_tests math_test.cc geometry_test.cc)
target_link_libraries(raytracer_tests PRIVATE raytracer_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(raytracer_tests)
