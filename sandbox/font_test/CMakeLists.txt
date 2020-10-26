
add_executable(font_test "font_test.cpp")



# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(font_test PUBLIC
/W4
#/WX
#/wd4251 #dll interface shit
#/wd4100 #unused variables
#/wd6011
/MP)

target_compile_features(font_test PUBLIC cxx_std_20)
target_link_libraries(font_test PUBLIC Destral)


