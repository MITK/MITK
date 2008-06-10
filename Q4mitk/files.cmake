SET(CPP_FILES
 qclickablelabel.cpp
 QmitkAbortEventFilter.cpp
 QmitkApplicationCursor.cpp
 QmitkCallbackFromGUIThread.cpp
 QmitkRegisterClasses.cpp
 QmitkRenderingManager.cpp
 QmitkRenderingManagerFactory.cpp
 QmitkRenderWindow.cpp
 
 QmitkEventAdapter.cpp
 
 QmitkStandardViews.cpp
 QmitkStepperAdapter.cpp
 QmitkLineEditLevelWindowWidget.cpp
 QmitkSliderLevelWindowWidget.cpp
 QmitkLevelWindowWidget.cpp
 QmitkSliderNavigatorWidget.cpp
 QmitkLevelWindowRangeChangeDialog.cpp
 QmitkLevelWindowPresetDefinitionDialog.cpp
 QmitkLevelWindowWidgetContextMenu.cpp
 QmitkSliceWidget.cpp
 QmitkStdMultiWidget.cpp
)

QT4_ADD_RESOURCES(CPP_FILES QmitkResources.qrc)


SET(MOC_H_FILES
 qclickablelabel.h
 QmitkCallbackFromGUIThread.h
 QmitkRenderingManager.h
 
 QmitkStandardViews.h
 QmitkStepperAdapter.h
 QmitkLineEditLevelWindowWidget.h
 QmitkSliderLevelWindowWidget.h
 QmitkSliderNavigatorWidget.h
 QmitkLevelWindowWidget.h
 QmitkLevelWindowRangeChangeDialog.h
 QmitkLevelWindowPresetDefinitionDialog.h
 QmitkLevelWindowWidgetContextMenu.h
 QmitkSliceWidget.h
 QmitkStdMultiWidget.h
)

SET(UI_FILES
  QmitkSliderNavigator.ui
  QmitkLevelWindowRangeChange.ui
  QmitkLevelWindowPresetDefinition.ui
  QmitkLevelWindowWidget.ui
  QmitkSliceWidget.ui
  QmitkStdMultiWidget.ui
)
