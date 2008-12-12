SET(SRC_CPP_FILES
  QmitkInteractiveSegmentationView.cpp
  QmitkToolGUI.cpp
  QmitkBinaryThresholdToolGUI.cpp
  #QmitkNewSegmentationDialog.cpp
  QmitkToolSelectionBox.cpp
  QmitkCalculateGrayValueStatisticsToolGUI.cpp
  QmitkDrawPaintbrushToolGUI.cpp
  QmitkPaintbrushToolGUI.cpp
  QmitkErasePaintbrushToolGUI.cpp
  #QmitkToolGUIArea.cpp
)

SET(INTERNAL_CPP_FILES
 
)

SET(UI_FILES
  src/QmitkInteractiveSegmentationControls.ui
)

SET(MOC_H_FILES
  src/QmitkInteractiveSegmentationView.h
  src/QmitkToolGUI.h
  src/QmitkBinaryThresholdToolGUI.h
  src/QmitkToolSelectionBox.h
  src/QmitkCalculateGrayValueStatisticsToolGUI.h
  src/QmitkDrawPaintbrushToolGUI.h
  src/QmitkPaintbrushToolGUI.h
  src/QmitkErasePaintbrushToolGUI.h
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
