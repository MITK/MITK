set(MODULE_TESTS
)

if(MITK_ENABLE_RENDERING_TESTING)
  list(APPEND MODULE_TESTS
    mitkSplineVtkMapper3DTest.cpp
  )
endif()
