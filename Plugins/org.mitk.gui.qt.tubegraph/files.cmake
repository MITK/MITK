SET(SRC_CPP_FILES

)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  mitkTubeGraphDefaultLabelGroups.cpp
  QmitkTubeGraphView.cpp
  QmitkTubeGraphLabelWidget.cpp
  QmitkTubeGraphLabelGroupWidget.cpp
  QmitkTubeGraphDeleteLabelGroupDialog.cpp
  QmitkTubeGraphNewLabelGroupDialog.cpp
  QmitkTubeGraphNewAnnotationDialog.cpp
)
SET(UI_FILES
  src/internal/QmitkTubeGraphViewControls.ui
)

SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkTubeGraphView.h
  src/internal/QmitkTubeGraphLabelWidget.h
  src/internal/QmitkTubeGraphLabelGroupWidget.h
  src/internal/QmitkTubeGraphDeleteLabelGroupDialog.h
  src/internal/QmitkTubeGraphNewLabelGroupDialog.h
  src/internal/QmitkTubeGraphNewAnnotationDialog.h
)

SET(CACHED_RESOURCE_FILES
  resources/tubeGraph.png
  resources/icon_seedpoint.png
  plugin.xml
)

SET(QRC_FILES

)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
