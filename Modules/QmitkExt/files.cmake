SET(CPP_FILES

 QmitkApplicationBase/QmitkCommonFunctionality.cpp
 
 #QmitkModels/QmitkDataStorageListModel.cpp
 #QmitkModels/QmitkPropertiesTableModel.cpp
 #QmitkModels/QmitkDataStorageTreeModel.cpp
 #QmitkModels/QmitkDataStorageTableModel.cpp
 #QmitkModels/QmitkPropertyDelegate.cpp
 #QmitkModels/QmitkPointListModel.cpp

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

 qclickablelabel.cpp
 #QmitkAbortEventFilter.cpp
 # QmitkApplicationCursor.cpp
 QmitkCallbackFromGUIThread.cpp
 QmitkExtRegisterClasses.cpp
 # QmitkRenderingManager.cpp
 # QmitkRenderingManagerFactory.cpp
 # QmitkRenderWindow.cpp
 
 # QmitkEventAdapter.cpp
 
 QmitkColorTransferFunctionCanvas.cpp
 QmitkSlicesInterpolator.cpp
 QmitkStandardViews.cpp
 QmitkStepperAdapter.cpp
 # QmitkLineEditLevelWindowWidget.cpp
 # mitkSliderLevelWindowWidget.cpp
 # QmitkLevelWindowWidget.cpp
 # QmitkPointListWidget.cpp
 # QmitkPointListView.cpp
 QmitkPiecewiseFunctionCanvas.cpp
 QmitkSliderNavigatorWidget.cpp
 QmitkTransferFunctionCanvas.cpp
  #QmitkLevelWindowRangeChangeDialog.cpp
 
 #QmitkLevelWindowPresetDefinitionDialog.cpp
 # QmitkLevelWindowWidgetContextMenu.cpp
 QmitkSliceWidget.cpp
 # QmitkStdMultiWidget.cpp
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
 # QmitkMaterialEditor.cpp
 # QmitkMaterialShowcase.cpp
 # QmitkPropertiesTableEditor.cpp
 QmitkPrimitiveMovieNavigatorWidget.cpp
 # QmitkDataStorageComboBox.cpp
 
 QmitkHistogram.cpp
 QmitkHistogramWidget.cpp
 QmitkPlotWidget.cpp
 QmitkPlotDialog.cpp
)

QT4_ADD_RESOURCES(CPP_FILES resources/QmitkResources.qrc)


SET(MOC_H_FILES
 
 
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

 qclickablelabel.h
 QmitkCallbackFromGUIThread.h
 
 QmitkStandardViews.h
 QmitkStepperAdapter.h
 QmitkSliderNavigatorWidget.h
 QmitkSliceWidget.h
 QmitkSlicesInterpolator.h
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
 QmitkPrimitiveMovieNavigatorWidget.h
 QmitkHistogramWidget.h
)

SET(UI_FILES
  QmitkSliderNavigator.ui
  # QmitkLevelWindowRangeChange.ui
  # QmitkLevelWindowPresetDefinition.ui
  # QmitkLevelWindowWidget.ui
  QmitkSliceWidget.ui
  QmitkTransferFunctionWidget.ui
  QmitkSelectableGLWidget.ui
  QmitkPrimitiveMovieNavigatorWidget.ui
)

# TODO TODO auf Qt4 anpassen
SET(TOOL_QT4GUI_FILES
  QmitkBinaryThresholdToolGUI.cpp
  QmitkDrawPaintbrushToolGUI.cpp
  QmitkErasePaintbrushToolGUI.cpp
  QmitkCalculateGrayValueStatisticsToolGUI.cpp
)
