SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  QmitkPointSetInteractionView.cpp
  mitkPluginActivator.cpp
)

SET(UI_FILES
  src/internal/QmitkPointSetInteractionViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkPointSetInteractionView.h
)

SET(CACHED_RESOURCE_FILES
  resources/pointset_interaction.png
  plugin.xml
)

SET(QRC_FILES
  resources/QmitkPointSetInteractionView.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

