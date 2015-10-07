set(MODULE_TESTS
  mitkPythonTest.cpp
)

#TODO: temporarily disabled untill segfault is fixed (bug-19152)
#if(UNIX)
#  set(MODULE_TESTS ${MODULE_TESTS} mitkVtkPythonTest.cpp)
#endif()
#
#if(MITK_USE_OpenCV)
#  set(MODULE_TESTS ${MODULE_TESTS} mitkCvPythonTest.cpp)
#endif()
#
#if(MITK_USE_SimpleITK)
#  set(MODULE_TESTS ${MODULE_TESTS} mitkSimpleItkPythonTest.cpp)
#endif()

