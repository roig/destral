cmake_minimum_required(VERSION 3.1)
project(Destral)

#set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bins)

####### Destral library compilation

# compile dependencies
add_subdirectory(libs/SDL2-2.0.14) #compile SDL2 
add_subdirectory(libs/freetype-2.9.1) #compile freetype


# Edv source files list
file(GLOB_RECURSE DESTRAL_SOURCE_FILES "src/*.cpp" "src/*.hpp")
file(GLOB_RECURSE DESTRAL_PUBLIC_HEADER_FILES  "include/*.h" "include/*.hpp")

add_library(Destral ${DESTRAL_PUBLIC_HEADER_FILES} ${DESTRAL_SOURCE_FILES} )


target_link_libraries(Destral PRIVATE
	Opengl32

	PUBLIC
	SDL2
	freetype
	#LuaLib54
)

target_include_directories(Destral PUBLIC include  PRIVATE libs/freetype-2.9.1/include )

target_compile_features(Destral PUBLIC cxx_std_20)

#This is to define the Destral build defines per configuration
target_compile_definitions(Destral PUBLIC
    $<$<CONFIG:Debug>:DS_BUILD_DEBUG>
    $<$<CONFIG:Release>:DS_BUILD_RELEASE>
	$<$<CONFIG:RelWithDebInfo>:DS_BUILD_RELEASE>
)


if (MSVC)
    # warning level 4 and all warnings as errors
    target_compile_options(Destral PUBLIC
	/W4
	/WX
	#/wd4251 #dll interface shit
	/wd4100 #unused variables
	#/wd4101 #unused local variables
	#/wd6011
	/wd4204 #non-constant aggregate initializer
	/wd5054
	
	/MP)
else()
    # lots of warnings and all warnings as errors
    target_compile_options(Destral PUBLIC -Wall -Wextra -pedantic -Werror)
endif()







######## End Destral library compilation

add_subdirectory(sandbox)