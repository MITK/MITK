set(bundle_path "${CMAKE_INSTALL_PREFIX}/${_bundle_dest_dir}/../..")
get_filename_component(bundle_path ${bundle_path} REALPATH)

#############################
# (1) Fix Qt-related issues #
#############################

# Compile list of Qt frameworks in bundle
unset(qt_frameworks)
file(GLOB qt_framework_paths "${bundle_path}/Contents/Frameworks/Qt*.framework")
foreach(qt_framework_path ${qt_framework_paths})
  get_filename_component(qt_framework ${qt_framework_path} NAME_WE)
  list(APPEND qt_frameworks ${qt_framework})
endforeach()

# For each Qt framework, change the style of dependencies to other
# Qt frameworks from @executable_path to @rpath. The install name tool
# only changes existing dependencies.
foreach(qt_framework ${qt_frameworks})
  set(in "${bundle_path}/Contents/Frameworks/${qt_framework}.framework/Versions/5/${qt_framework}")
  foreach(other_qt_framework ${qt_frameworks})
    set(from "@executable_path/../Frameworks/${other_qt_framework}.framework/Versions/5/${other_qt_framework}")
    set(to "@rpath/${other_qt_framework}.framework/Versions/5/${other_qt_framework}")
    execute_process(COMMAND install_name_tool -change ${from} ${to} ${in})
  endforeach()
endforeach()

# Do the same for QtWebEngineProcess
set(qtwebengineprocess_path "${bundle_path}/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess")
foreach(qt_framework ${qt_frameworks})
  set(from "@executable_path/../Frameworks/${qt_framework}.framework/Versions/5/${qt_framework}")
  set(to "@rpath/${qt_framework}.framework/Versions/5/${qt_framework}")
  execute_process(COMMAND install_name_tool -change ${from} ${to} ${qtwebengineprocess_path})
endforeach()

# Add corresponding rpath entries to the actual application and QtWebEngineProcess.
# The install name tool returns an error if an entry is already present.
get_filename_component(app ${bundle_path} NAME_WE)
set(app_path "${bundle_path}/Contents/MacOS/${app}")
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../Frameworks" ${app_path} ERROR_QUIET)
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../../../../.." ${qtwebengineprocess_path} ERROR_QUIET)

##################################################
# (2) Fix hard dependencies to auto-load modules #
##################################################

# Create symlinks to auto-load modules in MitkCore directory
file(GLOB autoload_module_paths "${bundle_path}/Contents/MacOS/MitkCore/*.dylib")
foreach(autoload_module_path ${autoload_module_paths})
  get_filename_component(autoload_module ${autoload_module_path} NAME)
  execute_process(COMMAND ln -s MitkCore/${autoload_module} WORKING_DIRECTORY "${bundle_path}/Contents/MacOS")
endforeach()
