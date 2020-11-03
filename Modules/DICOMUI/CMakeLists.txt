include_directories(${CTK_INCLUDE_DIRS})

MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKDICOMWidgets Qt5|Sql
    PRIVATE Qt5|OpenGL+Widgets+Xml
  )
