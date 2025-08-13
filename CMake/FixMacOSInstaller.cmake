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
  set(in "${bundle_path}/Contents/Frameworks/${qt_framework}.framework/Versions/A/${qt_framework}")
  foreach(other_qt_framework ${qt_frameworks})
    set(from "@executable_path/../Frameworks/${other_qt_framework}.framework/Versions/A/${other_qt_framework}")
    set(to "@rpath/${other_qt_framework}.framework/Versions/A/${other_qt_framework}")
    execute_process(COMMAND install_name_tool -change ${from} ${to} ${in} ERROR_QUIET)
  endforeach()
endforeach()

# Do the same for QtWebEngineProcess
set(qtwebenginecore_path "${bundle_path}/Contents/Frameworks/QtWebEngineCore.framework")
set(qtwebenginecore_helpers_path "${qtwebenginecore_path}/Versions/A/Helpers")
set(qtwebenginecore_helpers_symlink "${qtwebenginecore_path}/Helpers")
set(qtwebengineprocess_path "${qtwebenginecore_helpers_path}/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess")
if(EXISTS "${qtwebengineprocess_path}")
  foreach(qt_framework ${qt_frameworks})
    set(from "@executable_path/../Frameworks/${qt_framework}.framework/Versions/A/${qt_framework}")
    set(to "@rpath/${qt_framework}.framework/Versions/A/${qt_framework}")
    execute_process(COMMAND install_name_tool -change ${from} ${to} ${qtwebengineprocess_path} ERROR_QUIET)
  endforeach()

  if(NOT IS_SYMLINK "${qtwebenginecore_helpers_symlink}")
    # Create a symlink to the Helpers directory at the top level of the QtWebEngineCore framework
    execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink
      "Versions/Current/Helpers"
      "${qtwebenginecore_helpers_symlink}"
    )
  endif()
endif()

# Add corresponding rpath entries to the actual application and QtWebEngineProcess.
# The install name tool returns an error if an entry is already present.
get_filename_component(app ${bundle_path} NAME_WE)
set(app_path "${bundle_path}/Contents/MacOS/${app}")
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../Frameworks" ${app_path} ERROR_QUIET)
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../../../../../../.." ${qtwebengineprocess_path} ERROR_QUIET)

################################################
# (2) Use @loader_path dependencies for pyMITK #
################################################

file(GLOB lib_dirs "${bundle_path}/Contents/Resources/python/lib/python3.*")
foreach(lib_dir IN LISTS lib_dirs)
  if(IS_DIRECTORY "${lib_dir}")
    set(pymitk_dir "${lib_dir}/site-packages/pyMITK")
    if(EXISTS "${pymitk_dir}/_pyMITK.so")
      message("Use @loader_path dependencies for pyMITK...")
      file(GET_RUNTIME_DEPENDENCIES
        LIBRARIES "${pymitk_dir}/_pyMITK.so"
        BUNDLE_EXECUTABLE "${app_path}"
        PRE_EXCLUDE_REGEXES "^/"
        RESOLVED_DEPENDENCIES_VAR deps
      )
      foreach(dep IN LISTS deps)
        get_filename_component(dep_path "${dep}" REALPATH)
        get_filename_component(dep_name "${dep}" NAME)
        message("  ${dep_name}")
        foreach(other_dep IN LISTS deps)
          get_filename_component(other_dep_name "${other_dep}" NAME)
          set(from "@executable_path/../MacOS/${other_dep_name}")
          set(to "@loader_path/${other_dep_name}")
          execute_process(COMMAND install_name_tool -change "${from}" "${to}" "${dep_path}" ERROR_QUIET)
        endforeach()
        # TODO: Autoload modules
      endforeach()
      break()
    endif()
  endif()
endforeach()

###############################
# (3) Thin universal binaries #
###############################

file(GLOB_RECURSE all_files LIST_DIRECTORIES FALSE "${bundle_path}/*")
message("Thinning universal binaries to ${_osx_arch}...")
foreach(file_path IN LISTS all_files)
  execute_process(COMMAND file "${file_path}" OUTPUT_VARIABLE file_info)
  if(file_info MATCHES "universal")
    execute_process(COMMAND lipo -info "${file_path}" OUTPUT_VARIABLE lipo_info)
    if(lipo_info MATCHES "${_osx_arch}")
      execute_process(COMMAND lipo -thin "${_osx_arch}" "${file_path}" -output "${file_path}")
    endif()
  endif()
endforeach()
