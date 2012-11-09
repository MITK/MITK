set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkPointSetInteractionView.cpp
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkPointSetInteractionViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkPointSetInteractionView.h
)

set(CACHED_RESOURCE_FILES
  resources/pointset_interaction.png
  plugin.xml
)

set(QRC_FILES
  resources/QmitkPointSetInteractionView.qrc
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

