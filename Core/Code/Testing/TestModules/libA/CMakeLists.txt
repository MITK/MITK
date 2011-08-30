
MITK_CREATE_MODULE(TestModuleA
  DEPENDS Mitk)

add_dependencies(${TESTDRIVER} TestModuleA)
