MITK_INSTALL(FILES "${MITK_SOURCE_DIR}/mitk.ico")
MITK_INSTALL(FILES "${MITK_SOURCE_DIR}/mitk.bmp")

# Install CTK Qt (designer) plugins
if(MITK_USE_CTK)
  if(EXISTS ${CTK_QTDESIGNERPLUGINS_DIR})
    set(_qtplugin_install_destinations)
    if(MACOSX_BUNDLE_NAMES)
      foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
        list(APPEND _qtplugin_install_destinations
             ${bundle_name}.app/Contents/MacOS/${_install_DESTINATION}/plugins/designer)
      endforeach()
    else()
      list(APPEND _qtplugin_install_destinations bin/plugins/designer)
    endif()

    set(_ctk_qt_plugin_folder_release)
    set(_ctk_qt_plugin_folder_debug)
    if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
      set(_ctk_qt_plugin_folder_release "Release/")
      set(_ctk_qt_plugin_folder_debug "Debug/")
    endif()

    foreach(_qtplugin_install_dir ${_qtplugin_install_destinations})
      install(DIRECTORY "${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_ctk_qt_plugin_folder_release}"
              DESTINATION ${_qtplugin_install_dir}
              CONFIGURATIONS Release
              )
      install(DIRECTORY "${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_ctk_qt_plugin_folder_debug}"
              DESTINATION ${_qtplugin_install_dir}
              CONFIGURATIONS Debug
              )
    endforeach()
  endif()
endif()

# related to MITK:T19679
if(MACOSX_BUNDLE_NAMES)
  foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
    get_property(_qmake_location TARGET ${Qt5Core_QMAKE_EXECUTABLE}
                 PROPERTY IMPORT_LOCATION)
    get_filename_component(_qmake_path "${_qmake_location}" DIRECTORY)
    install(FILES "${_qmake_path}/../plugins/platforms/libqcocoa.dylib"
            DESTINATION "${bundle_name}.app/Contents/MacOS/platforms"
            CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/sqldrivers/libqsqlite.dylib"
            DESTINATION "${bundle_name}.app/Contents/MacOS/sqldrivers"
            CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/iconengines/libqsvgicon.dylib"
            DESTINATION "${bundle_name}.app/Contents/MacOS/iconengines"
            CONFIGURATIONS Release)
    # related to MITK:T19679-InstallQtWebEnginProcess
    if(MITK_USE_QT_WEBENGINE)
        get_filename_component(ABS_DIR_HELPERS "${_qmake_path}/../lib/QtWebEngineCore.framework/Helpers" REALPATH)
        install(DIRECTORY ${ABS_DIR_HELPERS}
                DESTINATION "${bundle_name}.app/Contents/Frameworks/QtWebEngineCore.framework/"
                CONFIGURATIONS Release)
    endif()
  endforeach()
endif()

if(WIN32)
  if(MITK_USE_QT)
    get_property(_qmake_location TARGET ${Qt5Core_QMAKE_EXECUTABLE}
                 PROPERTY IMPORT_LOCATION)
    get_filename_component(_qmake_path "${_qmake_location}" DIRECTORY)
    install(FILES "${_qmake_path}/../plugins/platforms/qwindows.dll"
            DESTINATION "bin/plugins/platforms"
            CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/sqldrivers/qsqlite.dll"
            DESTINATION "bin/plugins/sqldrivers"
            CONFIGURATIONS Release)
    install(FILES "${_qmake_path}/../plugins/imageformats/qsvg.dll"
            DESTINATION "bin/plugins/imageformats"
            CONFIGURATIONS Release)
    if(MITK_USE_QT_WEBENGINE)
      MITK_INSTALL( FILES "${_qmake_path}/QtWebEngineProcess.exe")
    endif()
    if (MITK_USE_QT_WEBENGINE)
      install(DIRECTORY "${_qmake_path}/../resources/"
              DESTINATION "bin/resources/"
              CONFIGURATIONS Release)
    endif()
    install(FILES "${_qmake_path}/../plugins/platforms/qwindowsd.dll"
            DESTINATION "bin/plugins/platforms"
            CONFIGURATIONS Debug)
    install(FILES "${_qmake_path}/../plugins/sqldrivers/qsqlited.dll"
            DESTINATION "bin/plugins/sqldrivers"
            CONFIGURATIONS Debug)
    install(FILES "${_qmake_path}/../plugins/imageformats/qsvgd.dll"
            DESTINATION "bin/plugins/imageformats"
            CONFIGURATIONS Debug)
    if (MITK_USE_QT_WEBENGINE)
      install(DIRECTORY "${_qmake_path}/../resources/"
              DESTINATION "bin/resources/"
              CONFIGURATIONS Debug)
    endif()
  endif()

  #DCMTK Dlls install target (shared libs on gcc only)
  if(MINGW AND DCMTK_ofstd_LIBRARY)
    set(_dcmtk_libs
        ${DCMTK_dcmdata_LIBRARY}
        ${DCMTK_dcmimgle_LIBRARY}
        ${DCMTK_dcmnet_LIBRARY}
        ${DCMTK_ofstd_LIBRARY}
       )

    foreach(_dcmtk_lib ${_dcmtk_libs})
      MITK_INSTALL(FILES ${_dcmtk_lib} )
    endforeach()
  endif()

  #MinGW dll
  if(MINGW)
    find_library(MINGW_RUNTIME_DLL "mingwm10.dll" HINTS ${CMAKE_FIND_ROOT_PATH}/sys-root/mingw/bin)
    if(MINGW_RUNTIME_DLL)
      MITK_INSTALL(FILES ${MINGW_RUNTIME_DLL} )
    else()
      message(SEND_ERROR "Could not find mingwm10.dll which is needed for a proper install")
    endif()

    find_library(MINGW_GCC_RUNTIME_DLL "libgcc_s_dw2-1.dll" HINTS ${CMAKE_FIND_ROOT_PATH}/sys-root/mingw/bin)
    if(MINGW_GCC_RUNTIME_DLL)
      MITK_INSTALL(FILES ${MINGW_GCC_RUNTIME_DLL} )
    else()
      message(SEND_ERROR "Could not find libgcc_s_dw2-1.dll which is needed for a proper install")
    endif()
  endif()

else()

  #DCMTK Dlls install target (shared libs on gcc only)
  if(DCMTK_ofstd_LIBRARY)
    set(_dcmtk_libs
        ${DCMTK_dcmdata_LIBRARY}
        ${DCMTK_dcmimgle_LIBRARY}
        ${DCMTK_dcmnet_LIBRARY}
        ${DCMTK_ofstd_LIBRARY}
    )
    foreach(_dcmtk_lib ${_dcmtk_libs})
      #MITK_INSTALL(FILES ${_dcmtk_lib} DESTINATION lib)
    endforeach()
  endif()

endif()
