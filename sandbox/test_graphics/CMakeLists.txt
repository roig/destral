cmake_minimum_required(VERSION 3.15)

#find_package(SDL2)


add_executable(test_graphics "test_graphics.cpp")


# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(test_graphics PUBLIC
/W4
#/WX
#/wd4251 #dll interface shit
#/wd4100 #unused variables
#/wd6011
/MP)

target_compile_features(test_graphics PUBLIC cxx_std_20)
target_link_libraries(test_graphics PUBLIC Destral)

# For convenience set MSVC debugger's working directory in the build folder.
# Also ask MSVC to make snake the startup project.
#if (MSVC)
	set_target_properties(test_graphics PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:test_graphics>")
#endif()
