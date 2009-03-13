SET(CPP_FILES

 QmitkApplicationBase/QmitkCommonFunctionality.cpp
 
 QmitkModels/QmitkDataStorageListModel.cpp
 QmitkModels/QmitkPropertiesTableModel.cpp
 QmitkModels/QmitkDataStorageTreeModel.cpp
 QmitkModels/QmitkDataStorageTableModel.cpp
 QmitkModels/QmitkPropertyDelegate.cpp

 QmitkPropertyObservers/QmitkBasePropertyView.cpp
 QmitkPropertyObservers/QmitkBoolPropertyEditor.cpp
 QmitkPropertyObservers/QmitkBoolPropertyView.cpp
 QmitkPropertyObservers/QmitkColorPropertyEditor.cpp
 QmitkPropertyObservers/QmitkColorPropertyView.cpp
 QmitkPropertyObservers/QmitkDataTreeComboBox.cpp
 QmitkPropertyObservers/QmitkNumberPropertyEditor.cpp
 QmitkPropertyObservers/QmitkNumberPropertyView.cpp
 QmitkPropertyObservers/QmitkPropertyViewFactory.cpp
 QmitkPropertyObservers/QmitkStringPropertyEditor.cpp
 QmitkPropertyObservers/QmitkStringPropertyOnDemandEdit.cpp
 QmitkPropertyObservers/QmitkStringPropertyView.cpp
 QmitkPropertyObservers/QmitkNumberPropertySlider.cpp
 QmitkPropertyObservers/QmitkDataStorageComboBox.cpp

 qclickablelabel.cpp
 QmitkAbortEventFilter.cpp
 QmitkApplicationCursor.cpp
 QmitkCallbackFromGUIThread.cpp
 QmitkRegisterClasses.cpp
 QmitkRenderingManager.cpp
 QmitkRenderingManagerFactory.cpp
 QmitkRenderWindow.cpp
 
 QmitkEventAdapter.cpp
 
 QmitkColorTransferFunctionCanvas.cpp
 QmitkSlicesInterpolator.cpp
 QmitkStandardViews.cpp
 QmitkStepperAdapter.cpp
 QmitkLineEditLevelWindowWidget.cpp
 QmitkSliderLevelWindowWidget.cpp
 QmitkLevelWindowWidget.cpp
 QmitkPiecewiseFunctionCanvas.cpp
 QmitkSliderNavigatorWidget.cpp
 QmitkTransferFunctionCanvas.cpp
 QmitkLevelWindowRangeChangeDialog.cpp
 QmitkLevelWindowPresetDefinitionDialog.cpp
 QmitkLevelWindowWidgetContextMenu.cpp
 QmitkSliceWidget.cpp
 QmitkStdMultiWidget.cpp
 QmitkTransferFunctionWidget.cpp
 QmitkSelectableGLWidget.cpp
 QmitkToolReferenceDataSelectionBox.cpp
 QmitkToolGUIArea.cpp
 QmitkToolWorkingDataSelectionBox.cpp
 QmitkToolSelectionBox.cpp
 QmitkPropertyListPopup.cpp
 QmitkToolGUI.cpp
 QmitkNewSegmentationDialog.cpp
 QmitkPaintbrushToolGUI.cpp
 QmitkCopyToClipBoardDialog.cpp
 QmitkMaterialEditor.cpp
 QmitkMaterialShowcase.cpp
 QmitkPropertiesTableEditor.cpp
 QmitkPrimitiveMovieNavigatorWidget.cpp
)

QT4_ADD_RESOURCES(CPP_FILES resources/QmitkResources.qrc)


SET(MOC_H_FILES
 
 QmitkModels/QmitkDataStorageTableModel.h
 QmitkModels/QmitkPropertyDelegate.h
 
 QmitkPropertyObservers/QmitkBasePropertyView.h
 QmitkPropertyObservers/QmitkBoolPropertyEditor.h
 QmitkPropertyObservers/QmitkBoolPropertyView.h
 QmitkPropertyObservers/QmitkColorPropertyEditor.h
 QmitkPropertyObservers/QmitkColorPropertyView.h
 QmitkPropertyObservers/QmitkDataTreeComboBox.h
 QmitkPropertyObservers/QmitkNumberPropertyEditor.h
 QmitkPropertyObservers/QmitkNumberPropertyView.h
 QmitkPropertyObservers/QmitkPropertyViewFactory.h
 QmitkPropertyObservers/QmitkStringPropertyEditor.h
 QmitkPropertyObservers/QmitkStringPropertyOnDemandEdit.h
 QmitkPropertyObservers/QmitkStringPropertyView.h
 QmitkPropertyObservers/QmitkNumberPropertySlider.h
 QmitkPropertyObservers/QmitkDataStorageComboBox.h

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
 QmitkSlicesInterpolator.h
 QmitkStdMultiWidget.h
 QmitkColorTransferFunctionCanvas.h
 QmitkPiecewiseFunctionCanvas.h
 QmitkTransferFunctionCanvas.h
 QmitkTransferFunctionWidget.h
 QmitkToolGUIArea.h
 QmitkToolGUI.h
 QmitkToolReferenceDataSelectionBox.h  
 QmitkToolSelectionBox.h  
 QmitkPropertyListPopup.h
 QmitkToolWorkingDataSelectionBox.h  
 QmitkSelectableGLWidget.h  
 QmitkNewSegmentationDialog.h
 QmitkPaintbrushToolGUI.h
 QmitkCopyToClipBoardDialog.h
 QmitkMaterialEditor.h
 QmitkMaterialShowcase.h
 QmitkPropertiesTableEditor.h
 QmitkPrimitiveMovieNavigatorWidget.h
)

SET(UI_FILES
  QmitkSliderNavigator.ui
  QmitkLevelWindowRangeChange.ui
  QmitkLevelWindowPresetDefinition.ui
  QmitkLevelWindowWidget.ui
  QmitkSliceWidget.ui
  QmitkStdMultiWidget.ui
  QmitkTransferFunctionWidget.ui
  QmitkSelectableGLWidget.ui
  QmitkMaterialEditor.ui
  QmitkMaterialShowcase.ui
  QmitkPrimitiveMovieNavigatorWidget.ui
)

# TODO TODO auf Qt4 anpassen
SET(TOOL_QT4GUI_FILES
  QmitkBinaryThresholdToolGUI.cpp
  QmitkDrawPaintbrushToolGUI.cpp
  QmitkErasePaintbrushToolGUI.cpp
  QmitkCalculateGrayValueStatisticsToolGUI.cpp
)
