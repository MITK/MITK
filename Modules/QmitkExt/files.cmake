set(CPP_FILES

 QmitkApplicationBase/QmitkIOUtil.cpp

 QmitkAboutDialog/QmitkAboutDialog.cpp

 QmitkPropertyObservers/QmitkBasePropertyView.cpp
 QmitkPropertyObservers/QmitkBoolPropertyWidget.cpp
 QmitkPropertyObservers/QmitkColorPropertyEditor.cpp
 QmitkPropertyObservers/QmitkColorPropertyView.cpp
 QmitkPropertyObservers/QmitkEnumerationPropertyWidget.cpp
 QmitkPropertyObservers/QmitkNumberPropertyEditor.cpp
 QmitkPropertyObservers/QmitkNumberPropertyView.cpp
 QmitkPropertyObservers/QmitkPropertyViewFactory.cpp
 QmitkPropertyObservers/QmitkStringPropertyEditor.cpp
 QmitkPropertyObservers/QmitkStringPropertyOnDemandEdit.cpp
 QmitkPropertyObservers/QmitkStringPropertyView.cpp
 QmitkPropertyObservers/QmitkNumberPropertySlider.cpp
 QmitkPropertyObservers/QmitkUGCombinedRepresentationPropertyWidget.cpp

 qclickablelabel.cpp

 QmitkCallbackFromGUIThread.cpp
 QmitkEditPointDialog.cpp
 QmitkExtRegisterClasses.cpp
 QmitkFileChooser.cpp

 QmitkFloatingPointSpanSlider.cpp
 QmitkColorTransferFunctionCanvas.cpp
 QmitkStandardViews.cpp
 QmitkStepperAdapter.cpp
 QmitkPiecewiseFunctionCanvas.cpp
 QmitkSliderNavigatorWidget.cpp
 QmitkTransferFunctionCanvas.cpp
 QmitkCrossWidget.cpp
 QmitkSliceWidget.cpp
 QmitkTransferFunctionWidget.cpp
 QmitkTransferFunctionGeneratorWidget.cpp
 QmitkSelectableGLWidget.cpp
 QmitkPrimitiveMovieNavigatorWidget.cpp

 QmitkHistogram.cpp
 QmitkHistogramWidget.cpp

 QmitkPlotWidget.cpp
 QmitkPlotDialog.cpp
 QmitkPointListModel.cpp
 QmitkPointListView.cpp
 QmitkPointListWidget.cpp
 QmitkPointListViewWidget.cpp
 QmitkCorrespondingPointSetsView.cpp
 QmitkCorrespondingPointSetsModel.cpp
 QmitkCorrespondingPointSetsWidget.cpp
 QmitkVideoBackground.cpp
 QmitkHotkeyLineEdit.cpp
 QmitkBoundingObjectWidget.cpp

 QmitkModuleTableModel.cpp
 QmitkModulesDialog.cpp

 QmitkHistogramJSWidget.cpp
 QmitkWebPage.cpp

 QmitkLineEdit.cpp
)

set(MOC_H_FILES
 QmitkPropertyObservers/QmitkBasePropertyView.h
 QmitkPropertyObservers/QmitkBoolPropertyWidget.h
 QmitkPropertyObservers/QmitkColorPropertyEditor.h
 QmitkPropertyObservers/QmitkColorPropertyView.h
 QmitkPropertyObservers/QmitkEnumerationPropertyWidget.h
 QmitkPropertyObservers/QmitkNumberPropertyEditor.h
 QmitkPropertyObservers/QmitkNumberPropertyView.h
 QmitkPropertyObservers/QmitkStringPropertyEditor.h
 QmitkPropertyObservers/QmitkStringPropertyOnDemandEdit.h
 QmitkPropertyObservers/QmitkStringPropertyView.h
 QmitkPropertyObservers/QmitkNumberPropertySlider.h
 QmitkPropertyObservers/QmitkUGCombinedRepresentationPropertyWidget.h

 qclickablelabel.h
 QmitkCallbackFromGUIThread.h
 QmitkEditPointDialog.h
 QmitkStandardViews.h
 QmitkStepperAdapter.h
 QmitkSliderNavigatorWidget.h
 QmitkSliceWidget.h
 QmitkColorTransferFunctionCanvas.h
 QmitkPiecewiseFunctionCanvas.h
 QmitkTransferFunctionCanvas.h
 QmitkFloatingPointSpanSlider.h
 QmitkCrossWidget.h
 QmitkTransferFunctionWidget.h
 QmitkTransferFunctionGeneratorWidget.h
 QmitkPrimitiveMovieNavigatorWidget.h
 QmitkPlotWidget.h
 QmitkPointListModel.h
 QmitkPointListView.h
 QmitkPointListWidget.h
 QmitkPointListViewWidget.h
 QmitkCorrespondingPointSetsView.h
 QmitkCorrespondingPointSetsModel.h
 QmitkCorrespondingPointSetsWidget.h
 QmitkHistogramWidget.h
 QmitkVideoBackground.h
 QmitkFileChooser.h
 QmitkHotkeyLineEdit.h
 QmitkAboutDialog/QmitkAboutDialog.h
 QmitkBoundingObjectWidget.h
 QmitkPlotWidget.h

 QmitkHistogramJSWidget.h
 QmitkWebPage.h

 QmitkLineEdit.h
)

set(UI_FILES
  QmitkSliderNavigator.ui
  QmitkSliceWidget.ui
  QmitkTransferFunctionWidget.ui
  QmitkTransferFunctionGeneratorWidget.ui
  QmitkSelectableGLWidget.ui
  QmitkPrimitiveMovieNavigatorWidget.ui
  QmitkAboutDialog/QmitkAboutDialogGUI.ui
)

set(QRC_FILES
  QmitkExt.qrc
  resources/QmitkResources.qrc
)
