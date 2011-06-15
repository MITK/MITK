SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkVolumetryView.cpp
  mitkVolumetryPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkVolumetryViewControls.ui
)

SET(MOC_H_FILES
  src/internal/QmitkVolumetryView.h
  src/internal/mitkVolumetryPluginActivator.h
)

SET(CACHED_RESOURCE_FILES
  resources/volumetryIcon.xpm
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkVolumetryView.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})