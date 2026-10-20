set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  QmitkReportEditor.cpp
  QmitkReportEditorInput.cpp
  QmitkReportsPluginActivator.cpp
  QmitkReportStore.cpp
)

if(MITK_HAS_RESTAPI)
  list(APPEND INTERNAL_CPP_FILES QmitkReportRestRoutes.cpp)
endif()

set(MOC_H_FILES
  src/internal/QmitkReportEditor.h
  src/internal/QmitkReportsPluginActivator.h
  src/internal/QmitkReportStore.h
)

set(CACHED_RESOURCE_FILES
  resources/Report.svg
  plugin.xml
)

set(QRC_FILES
  resources/Reports.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()
