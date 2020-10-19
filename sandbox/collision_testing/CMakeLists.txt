
add_executable(collision_test "collision_test.cpp")



# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(collision_test PUBLIC
/W4
#/WX
#/wd4251 #dll interface shit
#/wd4100 #unused variables
#/wd6011
/MP)

target_compile_features(collision_test PUBLIC cxx_std_20)
target_link_libraries(collision_test PUBLIC Destral)


