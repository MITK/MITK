SET(SRC_CPP_FILES
  cherryIQtPreferencePage.cpp
  cherryIQtStyleManager.cpp
  cherryQModelIndexObject.cpp
  cherryQtEditorPart.cpp
  cherryQtItemSelection.cpp
  cherryQtPreferences.cpp
  cherryQtSelectionProvider.cpp
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
  
  cherryQtControlWidget.cpp
  cherryQtDisplay.cpp
  cherryQtDnDTweaklet.cpp
  cherryQtFileImageDescriptor.cpp
  cherryQtGlobalEventFilter.cpp
  cherryQtIconImageDescriptor.cpp
  cherryQtImageTweaklet.cpp
  cherryQtMainWindowControl.cpp
  cherryQtMessageDialogTweaklet.cpp
  cherryQtMissingImageDescriptor.cpp
  cherryQtOpenPerspectiveAction.cpp
  cherryQtPluginActivator.cpp
  cherryQtSash.cpp
  cherryQtShell.cpp
  cherryQtShowViewAction.cpp
  cherryQtShowViewDialog.cpp
  cherryQtStyleManager.cpp
  cherryQtStylePreferencePage.cpp
  cherryQtTracker.cpp
  cherryQtWidgetController.cpp
  cherryQtWidgetsTweaklet.cpp
  cherryQtWidgetsTweakletImpl.cpp
  cherryQtWorkbenchPageTweaklet.cpp
  cherryQtWorkbenchTweaklet.cpp
)

SET(MOC_H_FILES
  src/cherryQtSelectionProvider.h
  
  src/internal/defaultpresentation/cherryNativeTabFolder.h
  src/internal/defaultpresentation/cherryNativeTabItem.h
  src/internal/defaultpresentation/cherryQCTabBar.h
  
  src/internal/cherryQtControlWidget.h
  src/internal/cherryQtDisplay.h
  src/internal/cherryQtGlobalEventFilter.h
  src/internal/cherryQtMainWindowControl.h
  src/internal/cherryQtOpenPerspectiveAction.h
  src/internal/cherryQtSash.h
  src/internal/cherryQtShowViewAction.h
  src/internal/cherryQtStylePreferencePage.h
  src/internal/cherryQtTracker.h
  src/internal/cherryQtWidgetsTweakletImpl.h
)

SET(UI_FILES
  src/internal/cherryQtShowViewDialog.ui
  src/internal/cherryQtStylePreferencePage.ui
  src/internal/cherryQtStatusPart.ui
)

SET(RES_FILES
  resources/org_opencherry_ui_qt.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
