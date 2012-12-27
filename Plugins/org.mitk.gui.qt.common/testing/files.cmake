set(MOC_H_FILES
  src/QmitkQtCommonTestSuite.h
  src/api/QmitkMockFunctionality.h

  src/internal/mitkPluginActivator.h
)

set(SRC_CPP_FILES

  QmitkQtCommonTestSuite.cpp

  # API Tests
  api/QmitkFunctionalityTest.cpp
  api/QmitkMockFunctionality.cpp
  api/QmitkUiApiTestSuite.cpp

)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
)

set(CPP_FILES )

set(RESOURCE_FILES

#  icons/view.gif
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

