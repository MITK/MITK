MITK_CREATE_MODULE_TESTS()

if(UNIX AND NOT APPLE)
  set(qt_platform -platform minimal)
else()
  unset(qt_platform)
endif()

mitkAddCustomModuleTest(QmitkAbstractNodeSelectionWidgetTest QmitkAbstractNodeSelectionWidgetTest ${qt_platform})
