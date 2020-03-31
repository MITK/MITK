MITK_CREATE_MODULE(
  INCLUDE_DIRS Qmitk
  DEPENDS MitkCore MitkChart MitkImageStatistics MitkQtWidgets
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
