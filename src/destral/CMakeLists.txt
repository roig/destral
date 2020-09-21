find_package(SDL2)

# Those headers are the public ones that should be installed
set(Destral_PUBLIC_HEADERS

"destral.h"


) 


# Those headers will only be shown in the IDE (should not be installed)
set(Destral_PRIVATE_HEADERS
	
)


set(Destral_SOURCES 
"header_only_impl.c" 
"destral.c" 

#"renderer.cpp"
#"transform.cpp"

)


add_library(Destral

${Destral_SOURCES}
${Destral_PUBLIC_HEADERS}
${Destral_PRIVATE_HEADERS}

)

# link with dependencies
target_link_libraries(Destral PUBLIC
	SDL2::SDL2
	Opengl32
	#LuaLib
	# nlohmann_json::nlohmann_json
	#ImguiLibrary
	# tmxlite
)

# Header only dependencies
######### Cute Headers
target_include_directories(Destral PUBLIC
	..
	${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/cute_headers
	${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/sokol
	${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/ap
	${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/flecs
	${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/cglm
)

#target_compile_features(Destral PUBLIC cxx_std_20)

if (MSVC)
    # warning level 4 and all warnings as errors
    target_compile_options(Destral PUBLIC
	/W4
	/WX
	#/wd4251 #dll interface shit
	/wd4100 #unused variables
	/wd4101 #unused local variables
	#/wd6011
	/wd4204 #non-constant aggregate initializer
	/MP)
else()
    # lots of warnings and all warnings as errors
    target_compile_options(Destral PUBLIC -Wall -Wextra -pedantic -Werror)
endif()


# warnings suppression Visual studio and multithreading compiling /MP
target_compile_options(Destral PUBLIC 
)


# debug postfix
set_target_properties(Destral PROPERTIES
    DEBUG_POSTFIX "-d"
	FOLDER "Destral"
)

