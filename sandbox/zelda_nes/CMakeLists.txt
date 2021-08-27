cmake_minimum_required(VERSION 3.15)

#find_package(SDL2)


add_executable(zelda_nes "zelda_nes.cpp")


# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(zelda_nes PUBLIC
/W4
#/WX
#/wd4251 #dll interface shit
#/wd4100 #unused variables
#/wd6011
/MP)

target_compile_features(zelda_nes PUBLIC cxx_std_20)
target_link_libraries(zelda_nes PUBLIC Destral)

# For convenience set MSVC debugger's working directory in the build folder.
# Also ask MSVC to make snake the startup project.
#if (MSVC)
	set_target_properties(zelda_nes PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:zelda_nes>")
#endif()
