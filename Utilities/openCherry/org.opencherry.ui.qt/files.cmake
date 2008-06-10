SET(CPP_FILES
  manifest.cpp
  cherryLogView.cpp
  cherryQtLogView.cpp
  cherryQtUIPlugin.cpp
  cherryQtWorkbench.cpp
  #cherryQtWorkbenchWindow.cpp
  cherryQtSimpleWorkbenchWindow.cpp
)

SET(INTERNAL_CPP_FILES
  cherryQtEditorPane.cpp
  cherryQtErrorView.cpp
  cherryQtShowViewAction.cpp
  cherryQtViewPane.cpp
  cherryQtPlatformLogModel.cpp
  cherryQtShowViewDialog.cpp
  cherryQtSimpleEditorAreaHelper.cpp
)

SET(MOC_H_FILES
  cherryQtSimpleWorkbenchWindow.h
  internal/cherryQtShowViewAction.h
  internal/cherryQtViewPane.h
)

SET(UI_FILES
  internal/cherryQtShowViewDialog.ui
  cherryQtLogView.ui
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})