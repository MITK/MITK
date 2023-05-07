set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_flow_segmentation_Activator.cpp
  QmitkSegmentationFlowControlView.cpp
  perspectives/QmitkFlowSegmentationPerspective.cpp
)

set(UI_FILES
  src/internal/QmitkSegmentationFlowControlView.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_flow_segmentation_Activator.h
  src/internal/QmitkSegmentationFlowControlView.h
  src/internal/perspectives/QmitkFlowSegmentationPerspective.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/icon.svg
  plugin.xml
  resources/perspectives/segmentation_icon.svg
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

