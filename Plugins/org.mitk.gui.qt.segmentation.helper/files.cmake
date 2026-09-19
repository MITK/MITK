set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  QmitkMultiLabelSegmentationNodeDescriptor.cpp
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkMultiLabelSegmentationNodeDescriptor.h
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
