project(org_blueberry_test)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE BERRY_TEST_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src src/harness
  )

target_link_libraries(${PROJECT_NAME} optimized CppUnit debug CppUnitd)
