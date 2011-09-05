
MITK_CREATE_MODULE(TestModuleA2
  DEPENDS Mitk)

add_dependencies(${TESTDRIVER} TestModuleA2)
