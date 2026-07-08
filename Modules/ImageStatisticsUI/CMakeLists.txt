MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkImageStatistics MitkQtWidgets MitkQtWidgetsExt
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
