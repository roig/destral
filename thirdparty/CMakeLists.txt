# add_subdirectory(lua53)

# ######### TMXLITE
# add_subdirectory(tmxlite/tmxlite)


# ######### stduuid
# target_include_directories(Endavant PUBLIC stduuid/include)

# ######### cppcodec
# target_include_directories(Endavant PUBLIC cppcodec)

# ############### imgui building

#add_library(ImguiLibrary STATIC
#	imgui/imconfig.h
#	imgui/imgui.cpp
#	imgui/imgui.h
#	imgui/imgui_demo.cpp
#	imgui/imgui_draw.cpp
#	imgui/imgui_internal.h
#	imgui/imgui_widgets.cpp
#	imgui/imstb_rectpack.h
#	imgui/imstb_textedit.h
#	imgui/imstb_truetype.h
#	imgui/misc/cpp/imgui_stdlib.h
#	imgui/misc/cpp/imgui_stdlib.cpp
#)
#
#
#target_include_directories(ImguiLibrary PUBLIC 
#	./imgui/
#)


# ############## json
# # Typically you don't care so much for a third party library's tests to be
# # run from your own project's code.
# set(JSON_BuildTests OFF CACHE INTERNAL "")
# set(JSON_MultipleHeaders ON CACHE INTERNAL "")

# # If you only include this third party in PRIVATE source files, you do not
# # need to install it when your main project gets installed.
# # set(JSON_Install OFF CACHE INTERNAL "")

# # Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# # unintended consequences that will break the build.  It's generally
# # discouraged (although not necessarily well documented as such) to use
# # include(...) for pulling in other CMake projects anyways.
# add_subdirectory(nlohman_json)



######## imgui magnum integration building as static library
# set(WITH_IMGUI ON CACHE BOOL "" FORCE)
# set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "${CMAKE_CURRENT_SOURCE_DIR}/imgui" FORCE)
# set(BUILD_STATIC ON CACHE BOOL "" FORCE)
# add_subdirectory(magnum-integration)



######## Box2d Compiling
#add_subdirectory(Box2D)

