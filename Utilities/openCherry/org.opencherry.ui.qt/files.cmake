SET(CPP_FILES
  manifest.cpp
  cherryLogView.cpp
  cherryQtLogView.cpp
  cherryQtUIPlugin.cpp
  cherryQtWorkbench.cpp
  cherryQtWorkbenchWindow.cpp
)

SET(INTERNAL_CPP_FILES
  cherryQtPlatformLogModel.cpp
  cherryQtShowViewDialog.cpp
)

SET(MOC_H_FILES
  cherryQtWorkbenchWindow.h
)

SET(UI_FILES
  internal/cherryQtShowViewDialog.ui
  cherryQtLogView.ui
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})