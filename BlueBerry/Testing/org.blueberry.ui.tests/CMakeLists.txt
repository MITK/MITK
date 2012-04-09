project(org_blueberry_ui_tests)

MACRO_CREATE_CTK_PLUGIN(
  EXPORT_DIRECTIVE BERRY_UI_TESTS
  TEST_PLUGIN
)

target_link_libraries(${PROJECT_NAME} optimized CppUnit debug CppUnitd)

add_dependencies(${PROJECT_NAME} org_blueberry_ui_qt)

MACRO_TEST_UIPLUGIN()
