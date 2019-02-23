set(MITK_CPACK_PACKAGE_EXECUTABLES "" CACHE INTERNAL "Collecting windows shortcuts to executables" FORCE)
set(MITK_DIR ${PROJECT_BINARY_DIR})
set(MITK_EXPORTS_FILE_INCLUDED 1)

if(MITK_USE_Qt5)

  if(MITK_USE_CTK)
    add_subdirectory(PluginGenerator)
  endif()

  if(MITK_USE_BLUEBERRY)

    foreach(MITK_EXTENSION_DIR ${MITK_DIR_PLUS_EXTENSION_DIRS})
      set(MITK_APPLICATIONS_EXTENSION_DIR ${MITK_EXTENSION_DIR}/Applications)
      get_filename_component(MITK_APPLICATIONS_EXTENSION_DIR ${MITK_APPLICATIONS_EXTENSION_DIR} ABSOLUTE)
      if(EXISTS ${MITK_APPLICATIONS_EXTENSION_DIR}/AppList.cmake)
        set(MITK_APPS "")
        include(${MITK_APPLICATIONS_EXTENSION_DIR}/AppList.cmake)
        foreach(mitk_app ${MITK_APPS})
          # extract target_dir and option_name
          string(REPLACE "^^" "\\;" target_info ${mitk_app})
          set(target_info_list ${target_info})
          list(GET target_info_list 0 target_dir)
          list(GET target_info_list 1 option_name)
          # check if the application is enabled
          if(${option_name} OR MITK_BUILD_ALL_APPS)
            add_subdirectory(${MITK_APPLICATIONS_EXTENSION_DIR}/${target_dir} ${target_dir})
          endif()
        endforeach()
      endif()
    endforeach()

  endif()

endif()
