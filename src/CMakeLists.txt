# Source directory CMakeLists.txt
add_subdirectory(gui)
add_subdirectory(cli)

# Common headers
install(FILES global.h DESTINATION include)
