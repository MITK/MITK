set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkDicomBrowser.cpp
  QmitkDicomDataEventPublisher.cpp
  QmitkDicomEventHandler.cpp
)

set(UI_FILES
  src/internal/QmitkDicomBrowser.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkDicomBrowser.h
  src/internal/QmitkDicomDataEventPublisher.h
  src/internal/QmitkDicomEventHandler.h
)

set(CACHED_RESOURCE_FILES
  resources/dicom.svg
  plugin.xml
)

set(QRC_FILES
  resources/dicom.qrc
)

unset(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  list(APPEND CPP_FILES src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  list(APPEND CPP_FILES src/internal/${file})
endforeach()
