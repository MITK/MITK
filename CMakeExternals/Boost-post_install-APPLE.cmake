message(STATUS "Fixing relative dependencies")

file(GLOB boost_dylibs ./libboost*.dylib)

foreach(in ${boost_dylibs})
  foreach(other_boost_dylib ${boost_dylibs})
    get_filename_component(from ${other_boost_dylib} NAME)
    set(to "@rpath/${from}")
    execute_process(COMMAND install_name_tool -change ${from} ${to} ${in})
  endforeach()
endforeach()
