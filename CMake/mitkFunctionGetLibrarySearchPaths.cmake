function(mitkFunctionGetLibrarySearchPaths search_path intermediate_dir)

  set(_search_dirs )
  get_property(_additional_paths GLOBAL PROPERTY MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS)
  if(_additional_paths)
    list(APPEND _search_dirs ${_additional_paths})
  endif()

  if(MITK_RUNTIME_PATH)
    string(REPLACE "%VS_BUILD_TYPE%" "${intermediate_dir}" _runtime_path "${MITK_RUNTIME_PATH}")
    list(APPEND _search_dirs ${_runtime_path})
    list(REMOVE_DUPLICATES _search_dirs)
    set(${search_path} ${_search_dirs} PARENT_SCOPE)
    return()
  endif()

  list(APPEND _search_dirs
    ${MITK_VTK_LIBRARY_DIRS}/${intermediate_dir}
    ${MITK_ITK_LIBRARY_DIRS}/${intermediate_dir}
    ${QT_LIBRARY_DIR}
    ${QT_LIBRARY_DIR}/../bin
    ${MITK_BINARY_DIR}/bin/${intermediate_dir}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${intermediate_dir}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/${intermediate_dir}
    )

  if(MITK_USE_Boost AND MITK_USE_Boost_LIBRARIES AND NOT MITK_USE_SYSTEM_Boost)
    list(APPEND _search_dirs ${Boost_LIBRARY__search_dirs})
  endif()
  if(GDCM_DIR)
    list(APPEND _search_dirs ${GDCM_DIR}/bin/${intermediate_dir})
  endif()
  if(OpenCV_DIR)
    list(APPEND _search_dirs ${OpenCV_DIR}/bin/${intermediate_dir})
  endif()
  if(SOFA_DIR)
    list(APPEND _search_dirs ${SOFA_DIR}/bin/${intermediate_dir})
  endif()
  if(MITK_USE_TOF_PMDO3 OR MITK_USE_TOF_PMDCAMCUBE OR MITK_USE_TOF_PMDCAMBOARD)
    list(APPEND _search_dirs ${MITK_PMD_SDK_DIR}/plugins)
  endif()

  if(MITK_USE_BLUEBERRY)
    list(APPEND _search_dirs ${CTK_RUNTIME_LIBRARY_DIRS}/${intermediate_dir})
    if(DEFINED CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY)
      if(IS_ABSOLUTE "${CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY}")
        list(APPEND _search_dirs "${CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY}/${intermediate_dir}")
      else()
        list(APPEND _search_dirs "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY}/${intermediate_dir}")
      endif()
    endif()
  endif()

  if(MITK_LIBRARY_DIRS)
    foreach(lib_dir ${MITK_LIBRARY_DIRS})
      list(APPEND _search_dirs "${lib_dir}/${intermediate_dir}")
    endforeach()
  endif()

  list(REMOVE_DUPLICATES _search_dirs)

  set(${search_path} ${_search_dirs} PARENT_SCOPE)

endfunction()