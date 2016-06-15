set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkToFUtilView.cpp
  QmitkToFDeviceGeneration.cpp
  QmitkToFScreenshotMaker.cpp
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkToFDeviceGenerationControls.ui
  src/internal/QmitkToFUtilViewControls.ui
  src/internal/QmitkToFScreenshotMakerControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkToFUtilView.h
  src/internal/QmitkToFDeviceGeneration.h
  src/internal/QmitkToFScreenshotMaker.h
  src/internal/mitkPluginActivator.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
  resources/icon_tof_settings.svg
  resources/icon_tof_util.svg
  resources/icon_tof_screenshot.svg
)

set(QRC_FILES
  resources/QmitkToFUtilView.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
