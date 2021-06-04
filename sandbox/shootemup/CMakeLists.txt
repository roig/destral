cmake_minimum_required(VERSION 3.1)

#find_package(SDL2)


add_executable(shootemup "shootemup.cpp")


# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(shootemup PUBLIC
/W4
#/WX
#/wd4251 #dll interface shit
#/wd4100 #unused variables
#/wd6011
/MP)

target_compile_features(shootemup PUBLIC cxx_std_20)
target_link_libraries(shootemup PUBLIC Destral)

