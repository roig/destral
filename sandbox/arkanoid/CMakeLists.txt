
add_executable(Arkanoid
"arkanoid.c"
)



# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(Arkanoid PUBLIC
/W4
#/WX
#/wd4251 #dll interface shit
#/wd4100 #unused variables
#/wd6011
/MP)

#target_compile_features(Arkanoid PUBLIC cxx_std_20)
target_link_libraries(Arkanoid PUBLIC Destral)


