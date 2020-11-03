message(STATUS "Make Boost libraries use RPATH")

file(GLOB boost_dylibs ./libboost*.dylib)

foreach(boost_dylib ${boost_dylibs})
  get_filename_component(boost_dylib ${boost_dylib} NAME)
  execute_process(COMMAND install_name_tool -id @rpath/${boost_dylib} ${boost_dylib})

  foreach(other_boost_dylib ${boost_dylibs})
    get_filename_component(other_boost_dylib ${other_boost_dylib} NAME)
    execute_process(COMMAND install_name_tool -change ${other_boost_dylib} @rpath/${other_boost_dylib} ${boost_dylib})
  endforeach()
endforeach()
