MITK_CREATE_MODULE_TESTS()

if(UNIX AND NOT APPLE)
  set(minimum_platform "-platform minimal")
else()
  unset(minimum_platform)
endif()

mitkAddCustomModuleTest(QmitkAbstractNodeSelectionWidgetTest QmitkAbstractNodeSelectionWidgetTest ${minimum_platform})
