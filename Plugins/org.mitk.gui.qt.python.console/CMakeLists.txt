# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
if(MITK_USE_Python)
  project(org_mitk_gui_qt_python_console)

  set(SWIG_PYTHON_WRAPPER _mitkCorePython SwigRuntimePython)

  MACRO_CREATE_MITK_CTK_PLUGIN(
    EXPORT_DIRECTIVE CONSOLE_EXPORT
    EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDENCIES QmitkExt CTK
  )

  include_directories("${MITK_WRAPPING_SOURCE_DIR}/CSwig" "${MITK_WRAPPING_BINARY_DIR}/CSwig/Core")

  target_link_libraries(org_mitk_gui_qt_python_console ${SWIG_PYTHON_WRAPPER})
  
else()
  message("MITK Python Console needs Python. Enable MITK_USE_PYTHON")
endif()