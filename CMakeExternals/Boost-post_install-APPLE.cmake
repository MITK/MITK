message(STATUS "Make Boost libraries use RPATH")

file(GLOB boost_dylibs ./libboost*.dylib)

foreach(boost_dylib ${boost_dylibs})
  get_filename_component(boost_dylib ${boost_dylib} NAME)
  execute_process(COMMAND install_name_tool -id @rpath/${boost_dylib} ${boost_dylib})
endforeach()
