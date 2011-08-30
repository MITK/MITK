MITK_CREATE_MODULE(TestModuleSL1
  DEPENDS Mitk)

add_dependencies(${TESTDRIVER} TestModuleSL1)
