SET(SRC_CPP_FILES
  QmitkToolGUI.cpp
  #QmitkNewSegmentationDialog.cpp
  QmitkToolSelectionBox.cpp
  #QmitkToolGUIArea.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkInteractiveSegmentationView.cpp
  QmitkBinaryThresholdToolGUI.cpp
  QmitkCalculateGrayValueStatisticsToolGUI.cpp
  QmitkDrawPaintbrushToolGUI.cpp
  QmitkPaintbrushToolGUI.cpp
  QmitkErasePaintbrushToolGUI.cpp
)

SET(UI_FILES
  src/internal/QmitkInteractiveSegmentationControls.ui
)

SET(MOC_H_FILES
  src/QmitkToolGUI.h
  src/QmitkToolSelectionBox.h
  
  src/internal/QmitkBinaryThresholdToolGUI.h
  src/internal/QmitkInteractiveSegmentationView.h
  src/internal/QmitkCalculateGrayValueStatisticsToolGUI.h
  src/internal/QmitkDrawPaintbrushToolGUI.h
  src/internal/QmitkPaintbrushToolGUI.h
  src/internal/QmitkErasePaintbrushToolGUI.h
)

SET(RES_FILES
  resources.qrc
)


SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
