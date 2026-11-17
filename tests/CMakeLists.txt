add_executable(raytracer_tests math_test.cc geometry_test.cc aabb_test.cc kdtree_test.cc)
target_link_libraries(raytracer_tests PRIVATE raytracer_core GTest::gtest_main)
target_compile_definitions(raytracer_tests PRIVATE
    TEST_SCENE_PATH="${PROJECT_SOURCE_DIR}/teapot_n_glass.obj")
include(GoogleTest)
gtest_discover_tests(raytracer_tests)
