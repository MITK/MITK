include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Step6
  ${CMAKE_CURRENT_SOURCE_DIR}/../Step7
)

mitk_create_executable(
  DEPENDS MitkQtWidgetsExt
)
