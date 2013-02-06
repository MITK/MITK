project(org_mitk_gui_qt_python)

if(MITK_USE_Python)
  MACRO_CREATE_MITK_CTK_PLUGIN(
    EXPORT_DIRECTIVE org_mitk_gui_qt_python_EXPORT
    EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDENCIES QmitkExt mitkPython)
else()
  message(STATUS "Please enable MITK_USE_Python before using the python plugin")
endif()
