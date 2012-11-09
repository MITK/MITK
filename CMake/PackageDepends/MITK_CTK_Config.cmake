# message("In package CTK config")
find_package(CTK REQUIRED)
list(APPEND ALL_INCLUDE_DIRECTORIES ${CTK_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${CTK_LIBRARIES})
link_directories(${CTK_LIBRARY_DIRS})


