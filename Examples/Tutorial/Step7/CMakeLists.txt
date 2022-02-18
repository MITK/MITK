include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Step6)

mitk_create_executable(
  DEPENDS MitkQtWidgetsExt
  PACKAGE_DEPENDS ITK|RegionGrowing
)
