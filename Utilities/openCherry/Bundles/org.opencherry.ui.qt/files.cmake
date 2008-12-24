SET(CPP_FILES
  manifest.cpp
  
  cherryQtEditorPart.cpp
  cherryQtViewPart.cpp
  
  # application
  application/cherryQtWorkbenchAdvisor.cpp
)

SET(INTERNAL_CPP_FILES

  defaultpresentation/cherryEmptyTabFolder.cpp
  defaultpresentation/cherryEmptyTabItem.cpp
  defaultpresentation/cherryNativeTabFolder.cpp
  defaultpresentation/cherryNativeTabItem.cpp
  defaultpresentation/cherryQCTabBar.cpp
  defaultpresentation/cherryQtWorkbenchPresentationFactory.cpp
  
  util/cherryAbstractTabFolder.cpp
  util/cherryLeftToRightTabOrder.cpp
  util/cherryPartInfo.cpp
  util/cherryPresentablePartFolder.cpp
  util/cherryReplaceDragHandler.cpp
  util/cherryTabbedStackPresentation.cpp
  util/cherryTabFolderEvent.cpp

  cherryLogView.cpp

  cherryQtControlWidget.cpp
  cherryQtDnDTweaklet.cpp
  cherryQtErrorView.cpp
  cherryQtLogView.cpp
  cherryQtMainWindowControl.cpp
  cherryQtSash.cpp
  cherryQtShell.cpp
  cherryQtShowViewAction.cpp
  cherryQtTracker.cpp
  cherryQtUIPlugin.cpp
  cherryQtPlatformLogModel.cpp
  cherryQtShowViewDialog.cpp
  cherryQtWidgetController.cpp
  cherryQtWidgetsTweaklet.cpp
  cherryQtWidgetsTweakletImpl.cpp
  cherryQtWorkbenchPageTweaklet.cpp
  cherryQtWorkbenchTweaklet.cpp
)

SET(MOC_H_FILES
  internal/defaultpresentation/cherryNativeTabFolder.h
  internal/defaultpresentation/cherryQCTabBar.h
  
  internal/cherryQtControlWidget.h
  internal/cherryQtMainWindowControl.h
  internal/cherryQtSash.h
  internal/cherryQtShowViewAction.h
  internal/cherryQtTracker.h
  internal/cherryQtWidgetsTweakletImpl.h
)

SET(UI_FILES
  internal/cherryQtShowViewDialog.ui
  internal/cherryQtLogView.ui
)

SET(RES_FILES
  resources.qrc
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})