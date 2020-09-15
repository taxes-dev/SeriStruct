include(CTest)
enable_testing()

add_executable (tests tests.cpp)

target_link_libraries (tests LINK_PUBLIC SeriStruct)
target_compile_features (tests PUBLIC cxx_std_17)

include (ParseAndAddCatchTests)
ParseAndAddCatchTests(tests)

add_custom_command(
     TARGET tests
     COMMENT "Run tests"
     POST_BUILD 
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
     COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --progress --output-on-failures
)