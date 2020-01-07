set(package_deps PACKAGE_DEPENDS PRIVATE)

MITK_CREATE_MODULE_TESTS(${package_deps})

if(UNIX AND NOT APPLE)
  #[[ The PythonQt library (dependency of MitkQtPython module) depends on Qt
      libraries without absolute paths. Use LD_LIBRARY_PATH as workaround.
      See T26955. ]]
  if(TARGET ${TESTDRIVER})
    get_target_property(ld_library_path Qt5::Core IMPORTED_LOCATION_RELEASE)
    get_filename_component(ld_library_path "${ld_library_path}" DIRECTORY)

    add_test(
      NAME mitkPythonTest
      COMMAND ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${ld_library_path}"
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTDRIVER} mitkPythonTest
    )
  endif()
endif()
