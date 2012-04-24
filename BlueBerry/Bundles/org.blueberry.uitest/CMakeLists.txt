project(org_blueberry_uitest)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE BERRY_UITEST_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src src/harness src/util
)

target_link_libraries(${PROJECT_NAME} optimized CppUnit debug CppUnitd)
