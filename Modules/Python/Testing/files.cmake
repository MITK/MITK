set(MODULE_TESTS
  mitkPythonTest.cpp
  mitkVtkPythonTest.cpp
)

if(MITK_USE_OpenCV)
  set(MODULE_TESTS ${MODULE_TESTS} mitkCvPythonTest.cpp)
endif()

if(MITK_USE_SimpleITK)
  set(MODULE_TESTS ${MODULE_TESTS} mitkSimpleItkPythonTest.cpp)
endif()

