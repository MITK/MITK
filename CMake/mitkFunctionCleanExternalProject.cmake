#[[ mitkCleanExternalProject(<ep_name>)

    The MITK superbuild script stores hashes of all external project scripts
    and compares them with previous hashes, if they exist. This comparison
    potentially detects changes in an external project that may require
    cleaning of previous build artifacts to prevent MITK from referring to
    mixed versions of the same external project.

    This function tries to mimic the manual cleaning of old build articafts
    by removing the source, build, and stamp directory of the external project
    as well as all files in the external project's install manifest, if found.

    The motivation for this function is to improve build times in continuous
    integration and to make non-nightly build clients more robust against
    external project updates.
 ]]
function(mitkCleanExternalProject ep_name)
  message(STATUS "Cleaning '${ep_name}'")

  set(source_dir "${ep_prefix}/src/${ep_name}")
  set(binary_dir "${source_dir}-build")
  set(stamp_dir "${source_dir}-stamp")

  set(install_manifest_file "${binary_dir}/install_manifest.txt")
  if(EXISTS ${install_manifest_file})
    message(STATUS "  Validating install manifest")
    string(LENGTH ${CMAKE_BINARY_DIR} cmake_binary_dir_length)
    file(STRINGS ${install_manifest_file} installed_files)
    foreach(f ${installed_files})
      string(SUBSTRING "${f}" 0 ${cmake_binary_dir_length} f_prefix)
      if(NOT f_prefix STREQUAL CMAKE_BINARY_DIR)
        message(FATAL_ERROR "'${f}' in ${install_manifest_file}' refers to a file outside of '${CMAKE_BINARY_DIR}'!")
      endif()
    endforeach()
    message(STATUS "  Uninstalling")
    file(REMOVE ${installed_files})
  endif()

  if(source_dir)
    message(STATUS "  Removing source directory")
    file(REMOVE_RECURSE ${source_dir})
  endif()

  if(binary_dir)
    message(STATUS "  Removing build directory")
    file(REMOVE_RECURSE ${binary_dir})
  endif()

  if(stamp_dir)
    message(STATUS "  Removing stamp directory")
    file(REMOVE_RECURSE ${stamp_dir})
  endif()
endfunction()
