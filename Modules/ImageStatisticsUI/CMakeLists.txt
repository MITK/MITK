MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkChart MitkImageStatistics MitkQtWidgets
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
