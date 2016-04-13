project(org_mitk_gui_qt_cmdlinemodules)

mitk_create_plugin(
  EXPORT_DIRECTIVE CLI_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS
    PUBLIC  CTK|CTKWidgets
    PRIVATE CTK|CTKCommandLineModulesFrontendQtGui+CTKCommandLineModulesBackendLocalProcess Qt5|UiTools+XmlPatterns
)

# Copy CTK Qt (designer) plugins to a Qt default path in the MITK bin
# this has been implemented as a fix for bug 19379
# The Qt plugins provided by CTK were not found in the plugin paths
# when running in the build tree
if(MITK_USE_CTK)
  if(EXISTS ${CTK_QTDESIGNERPLUGINS_DIR})

    if(MACOSX_BUNDLE_NAMES)
      set(_bindir ${MITK_BINARY_DIR}/bin/MitkWorkbench.app/Contents/MacOS/)
    else()
      set(_bindir ${MITK_BINARY_DIR}/bin)
    endif()

    set(_releasedir)
    set(_debugdir)
    if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
      set(_releasedir "Release/")
      set(_debugdir "Debug/")
    endif()

    if(EXISTS ${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_releasedir})
      file(COPY "${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_releasedir}" DESTINATION ${_bindir}/${_releasedir}designer/)
    endif()
    if(EXISTS ${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_debugdir})
      file(COPY "${CTK_QTDESIGNERPLUGINS_DIR}/designer/${_debugdir}" DESTINATION ${_bindir}/${_debugdir}designer/)
    endif()

  endif()
endif()
