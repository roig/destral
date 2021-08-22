# destral
Astral Pixel Game Development Framework


# Files, paths and extensions naming the function parameters char* 

These names will be used in parameters of functions that deal with files/paths/extensions etc..:

| name       | example          |
|------------|------------------|
| file_path  | /foo/bar/baa.txt |
| file_dir   | /foo/bar/        |
| file_name  | baa.txt          |
| file_stem  | baa              |
| file_ext   | .txt             |
| file_parent| /foo/bar/        | (useful for parent dirs)



# How to set current working directories for projects:
1) go to CMake targets view (solution explorer)
2) open launch.vs.json (right clikc and "ADd debug configuration" over the cmake executable project)
3) and add:  "currentDir": "${workspaceRoot}/sandbox/test_graphics"  point to the folder you need.