set(CPP_FILES

 QmitkApplicationBase/QmitkIOUtil.cpp

 #QmitkModels/QmitkDataStorageListModel.cpp
 #QmitkModels/QmitkPropertiesTableModel.cpp
 #QmitkModels/QmitkDataStorageTreeModel.cpp
 #QmitkModels/QmitkDataStorageTableModel.cpp
 #QmitkModels/QmitkPropertyDelegate.cpp
 #QmitkModels/QmitkPointListModel.cpp

 #QmitkAlgorithmFunctionalityComponent.cpp
 #QmitkBaseAlgorithmComponent.cpp
 QmitkAboutDialog/QmitkAboutDialog.cpp
        #QmitkFunctionalityComponents/QmitkSurfaceCreatorComponent.cpp
              #QmitkFunctionalityComponents/QmitkPixelGreyValueManipulatorComponent.cpp
                #QmitkFunctionalityComponents/QmitkConnectivityFilterComponent.cpp
            #QmitkFunctionalityComponents/QmitkImageCropperComponent.cpp
              #QmitkFunctionalityComponents/QmitkSeedPointSetComponent.cpp
    #QmitkFunctionalityComponents/QmitkSurfaceTransformerComponent.cpp

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
 #QmitkAbortEventFilter.cpp
 # QmitkApplicationCursor.cpp
 QmitkCallbackFromGUIThread.cpp
 QmitkEditPointDialog.cpp
 QmitkExtRegisterClasses.cpp
 QmitkFileChooser.cpp
 # QmitkRenderingManager.cpp
 # QmitkRenderingManagerFactory.cpp
 # QmitkRenderWindow.cpp

 # QmitkEventAdapter.cpp

 QmitkFloatingPointSpanSlider.cpp
 QmitkColorTransferFunctionCanvas.cpp
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
 QmitkCrossWidget.cpp
  #QmitkLevelWindowRangeChangeDialog.cpp

 #QmitkLevelWindowPresetDefinitionDialog.cpp
 # QmitkLevelWindowWidgetContextMenu.cpp
 QmitkSliceWidget.cpp
 # QmitkStdMultiWidget.cpp
 QmitkTransferFunctionWidget.cpp
 QmitkTransferFunctionGeneratorWidget.cpp
 QmitkSelectableGLWidget.cpp
 # QmitkPropertyListPopup.cpp
 # QmitkMaterialEditor.cpp
 # QmitkMaterialShowcase.cpp
 # QmitkPropertiesTableEditor.cpp
 QmitkPrimitiveMovieNavigatorWidget.cpp
 # QmitkDataStorageComboBox.cpp

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

 QmitkLineEdit.cpp
)

if( NOT ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${VTK_BUILD_VERSION} VERSION_LESS 5.4.0 )
  set(CPP_FILES
    ${CPP_FILES}
    QmitkVtkHistogramWidget.cpp
    QmitkVtkLineProfileWidget.cpp
  )
endif()

if(NOT APPLE)
set(CPP_FILES
  ${CPP_FILES}
 QmitkBaseComponent.cpp
 QmitkBaseFunctionalityComponent.cpp
 QmitkFunctionalityComponentContainer.cpp
 QmitkFunctionalityComponents/QmitkThresholdComponent.cpp

 )
endif()
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

              # QmitkFunctionalityComponents/QmitkSurfaceCreatorComponent.h
                      #QmitkFunctionalityComponents/QmitkPixelGreyValueManipulatorComponent.h
                  # QmitkFunctionalityComponents/QmitkConnectivityFilterComponent.h
            # QmitkFunctionalityComponents/QmitkImageCropperComponent.h
                                # QmitkFunctionalityComponents/QmitkSeedPointSetComponent.h
                        # QmitkFunctionalityComponents/QmitkSurfaceTransformerComponent.h

 qclickablelabel.h
 QmitkCallbackFromGUIThread.h
 QmitkEditPointDialog.h
 #QmitkAlgorithmFunctionalityComponent.h
 #QmitkBaseAlgorithmComponent.h
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
 # QmitkPropertyListPopup.h
 #QmitkSelectableGLWidget.h
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

 QmitkLineEdit.h
)

if( NOT ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${VTK_BUILD_VERSION} VERSION_LESS 5.4.0 )
  set(MOC_H_FILES
    ${MOC_H_FILES}
    QmitkVtkHistogramWidget.h
    QmitkVtkLineProfileWidget.h
  )
endif()

if(NOT APPLE)
set(MOC_H_FILES
  ${MOC_H_FILES}
 QmitkBaseComponent.h
 QmitkBaseFunctionalityComponent.h
 QmitkFunctionalityComponentContainer.h
 QmitkFunctionalityComponents/QmitkThresholdComponent.h
)
endif()

set(UI_FILES
  QmitkSliderNavigator.ui
  # QmitkLevelWindowRangeChange.ui
  # QmitkLevelWindowPresetDefinition.ui
  # QmitkLevelWindowWidget.ui
  QmitkSliceWidget.ui
  QmitkTransferFunctionWidget.ui
  QmitkTransferFunctionGeneratorWidget.ui
  QmitkSelectableGLWidget.ui
  QmitkPrimitiveMovieNavigatorWidget.ui
  QmitkFunctionalityComponentContainerControls.ui
  QmitkFunctionalityComponents/QmitkThresholdComponentControls.ui
  QmitkAboutDialog/QmitkAboutDialogGUI.ui
)


set(QRC_FILES
  QmitkExt.qrc
  resources/QmitkResources.qrc
)
