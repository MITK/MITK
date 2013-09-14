project(org_blueberry_test)

include_directories(${CppUnit_INCLUDE_DIRS})

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE BERRY_TEST_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src src/harness
  NO_INSTALL
  )

target_link_libraries(${PROJECT_NAME} ${CppUnit_LIBRARIES})
