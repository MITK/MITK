set(SRC_CPP_FILES
  berryIQtPreferencePage.cpp
  berryIQtStyleManager.cpp
  berryQCHPluginListener.cpp
  berryQtAssistantUtil.cpp
  berryQModelIndexObject.cpp
  berryQtEditorPart.cpp
  berryQtItemSelection.cpp
  berryQtIntroPart.cpp
  berryQtPreferences.cpp
  berryQtSelectionProvider.cpp
  berryQtViewPart.cpp

  # application
  application/berryQtWorkbenchAdvisor.cpp
)

set(INTERNAL_CPP_FILES

  defaultpresentation/berryEmptyTabFolder.cpp
  defaultpresentation/berryEmptyTabItem.cpp
  defaultpresentation/berryNativeTabFolder.cpp
  defaultpresentation/berryNativeTabItem.cpp
  defaultpresentation/berryQCTabBar.cpp
  defaultpresentation/berryQtWorkbenchPresentationFactory.cpp

  util/berryAbstractTabFolder.cpp
  util/berryAbstractTabItem.cpp
  util/berryIPresentablePartList.cpp
  util/berryLeftToRightTabOrder.cpp
  util/berryPartInfo.cpp
  util/berryPresentablePartFolder.cpp
  util/berryReplaceDragHandler.cpp
  util/berryTabbedStackPresentation.cpp
  util/berryTabDragHandler.cpp
  util/berryTabFolderEvent.cpp
  util/berryTabOrder.cpp

  berryQtDisplay.cpp
  berryQtDnDTweaklet.cpp
  berryQtFileImageDescriptor.cpp
  berryQtGlobalEventFilter.cpp
  berryQtIconImageDescriptor.cpp
  berryQtImageTweaklet.cpp
  berryQtMainWindowControl.cpp
  berryQtMessageDialogTweaklet.cpp
  berryQtMissingImageDescriptor.cpp
  berryQtOpenPerspectiveAction.cpp
  berryQtPerspectiveSwitcher.cpp
  berryQtPluginActivator.cpp
  berryQtSafeApplication.cpp
  berryQtSash.cpp
  berryQtShell.cpp
  berryQtShowViewAction.cpp
  berryQtShowViewDialog.cpp
  berryQtStyleManager.cpp
  berryQtStylePreferencePage.cpp
  berryQtTracker.cpp
  berryQtWidgetsTweaklet.cpp
  berryQtWidgetsTweakletImpl.cpp
  berryQtWorkbenchPageTweaklet.cpp
  berryQtWorkbenchTweaklet.cpp
  berryQtWorkbenchWindow.cpp
)

set(MOC_H_FILES
  src/berryQCHPluginListener.h
  src/berryQtSelectionProvider.h

  src/internal/defaultpresentation/berryNativeTabFolder.h
  src/internal/defaultpresentation/berryNativeTabItem.h
  src/internal/defaultpresentation/berryQCTabBar.h
  src/internal/defaultpresentation/berryQtWorkbenchPresentationFactory.h

  src/internal/berryQtDisplay.h
  src/internal/berryQtDnDTweaklet.h
  src/internal/berryQtGlobalEventFilter.h
  src/internal/berryQtImageTweaklet.h
  src/internal/berryQtMainWindowControl.h
  src/internal/berryQtMessageDialogTweaklet.h
  src/internal/berryQtOpenPerspectiveAction.h
  src/internal/berryQtPerspectiveSwitcher.h
  src/internal/berryQtSash.h
  src/internal/berryQtShowViewAction.h
  src/internal/berryQtStyleManager.h
  src/internal/berryQtStylePreferencePage.h
  src/internal/berryQtTracker.h
  src/internal/berryQtWidgetsTweaklet.h
  src/internal/berryQtWidgetsTweakletImpl.h
  src/internal/berryQtWorkbenchTweaklet.h
  src/internal/berryQtWorkbenchPageTweaklet.h

  src/internal/berryQtPluginActivator.h
)

set(UI_FILES
  src/internal/berryQtShowViewDialog.ui
  src/internal/berryQtStylePreferencePage.ui
  src/internal/berryQtStatusPart.ui
)

set(QRC_FILES
  resources/org_blueberry_ui_qt.qrc
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
