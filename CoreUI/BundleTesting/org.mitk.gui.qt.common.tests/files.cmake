SET(MOC_H_FILES
  src/QmitkQtCommonTestSuite.h
  src/api/QmitkMockFunctionality.h
  
  src/internal/mitkPluginActivator.h
)

SET(SRC_CPP_FILES

  QmitkQtCommonTestSuite.cpp

  # API Tests
  api/QmitkFunctionalityTest.cpp
  api/QmitkMockFunctionality.cpp
  api/QmitkUiApiTestSuite.cpp
  
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
)

SET(CPP_FILES )

SET(RESOURCE_FILES

#  icons/view.gif
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

