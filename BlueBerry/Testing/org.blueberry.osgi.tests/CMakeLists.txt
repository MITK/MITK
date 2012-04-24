project(org_blueberry_osgi_tests)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE BERRY_OSGI_TESTS
  TEST_PLUGIN
)

target_link_libraries(${PROJECT_NAME} optimized CppUnit debug CppUnitd)

MACRO_TEST_PLUGIN()
