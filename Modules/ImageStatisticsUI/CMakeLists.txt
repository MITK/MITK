MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Qmitk
  DEPENDS MitkCore MitkChart MitkImageStatistics MitkQtWidgets
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
