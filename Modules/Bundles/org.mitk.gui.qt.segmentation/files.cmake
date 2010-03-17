SET(SRC_CPP_FILES
  QmitkSegmentationPreferencePage.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkSegmentationView.cpp
  QmitkSegmentationPostProcessing.cpp
)

SET(UI_FILES
  src/internal/QmitkSegmentationControls.ui
)

SET(MOC_H_FILES
  src/QmitkSegmentationPreferencePage.h
  src/internal/QmitkSegmentationView.h
  src/internal/QmitkSegmentationPostProcessing.h
)

SET(RESOURCE_FILES
  resources/segmentation.png
)

SET(RES_FILES
  resources/segmentation.qrc
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
