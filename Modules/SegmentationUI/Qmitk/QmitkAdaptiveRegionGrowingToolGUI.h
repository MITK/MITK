/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef QMITK_QmitkAdaptiveRegionGrowingToolGUI_H
#define QMITK_QmitkAdaptiveRegionGrowingToolGUI_H


#include "mitkDataStorage.h"
#include "itkImage.h"
#include "mitkGeometry3D.h"
#include "mitkPointSet.h"

#include "qwidget.h"
#include "ui_QmitkAdaptiveRegionGrowingToolGUIControls.h"

#include "SegmentationUIExports.h"

#include "QmitkToolGUI.h"

#include "mitkAdaptiveRegionGrowingTool.h"

class QmitkStdMultiWidget;
class DataNode;
class QmitkAdaptiveRegionGrowingToolGUIControls;


/*!
*
* \brief QmitkAdaptiveRegionGrowingToolGUI
*
* Adaptive Region Growing View class of the segmentation.
*
*/

class SegmentationUI_EXPORT QmitkAdaptiveRegionGrowingToolGUI : public QmitkToolGUI
{

  Q_OBJECT

public:
  mitkClassMacro(QmitkAdaptiveRegionGrowingToolGUI, QmitkToolGUI);

  itkNewMacro(QmitkAdaptiveRegionGrowingToolGUI);

  QmitkAdaptiveRegionGrowingToolGUI(QWidget* parent=0);


  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();

  ///** \brief Method to set the default data storage.*/
  virtual void SetDataStorage(mitk::DataStorage* dataStorage);

  void SetMultiWidget(QmitkStdMultiWidget* multiWidget);

  void SetDataNodeNames(std::string labledSegmentation, std::string binaryImage, /*std::string vesselTree,*/ std::string surface);

  void EnableControls(bool enable);

  void SetInputImageNode(mitk::DataNode* node);

  void Deactivated();
  void Activated();


  /**
  * @brief The created GUI from the .ui-File. This Attribute is obligatory
  */
  Ui::QmitkAdaptiveRegionGrowingToolGUIControls m_Controls;

protected slots:

    void RunSegmentation();

    void ChangeLevelWindow(double newValue);//called, when the Level Window is changed via the slider in the ControlWidget

    //****called, when the slider-position is modified via the +/- buttons
    void IncreaseSlider();

    void DecreaseSlider();
    //***

    void ConfirmSegmentation();

    void UseVolumeRendering(bool on);

    void SetLowerThresholdValue(double lowerThreshold);

    void SetUpperThresholdValue(double upperThreshold);

    void OnNewToolAssociated(mitk::Tool*);

protected:

  mitk::AdaptiveRegionGrowingTool::Pointer m_RegionGrow3DTool;

  /** \brief Destructor. */
  virtual ~QmitkAdaptiveRegionGrowingToolGUI();

  //Pointer to the main widget to be able to reach the renderer
  QmitkStdMultiWidget* m_MultiWidget;

  mitk::DataStorage* m_DataStorage;

  mitk::DataNode::Pointer m_InputImageNode;

  void OnPointAdded();

private:

  std::string m_NAMEFORORGIMAGE;
  std::string m_NAMEFORLABLEDSEGMENTATIONIMAGE;
  std::string m_NAMEFORBINARYIMAGE;
  std::string m_NAMEFORSURFACE;

  mitk::ScalarType m_LOWERTHRESHOLD; //Hounsfield value
  mitk::ScalarType m_UPPERTHRESHOLD; //Hounsfield value
  mitk::ScalarType m_SeedPointValueMean;

  void RemoveHelperNodes();

  int m_DetectedLeakagePoint;

  bool m_CurrentRGDirectionIsUpwards; // defines fixed threshold (true = LOWERTHRESHOLD fixed, false = UPPERTHRESHOLD fixed)

  int m_SeedpointValue;
  bool m_SliderInitialized;
  bool m_UseVolumeRendering;
  bool m_UpdateSuggestedThreshold;
  float m_SuggestedThValue;

  long m_PointSetAddObserverTag;

  template < typename TPixel, unsigned int VImageDimension >
  void StartRegionGrowing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::BaseGeometry* imageGeometry, mitk::PointSet::PointType seedPoint );

  template < typename TPixel, unsigned int VImageDimension >
  void ITKThresholding( itk::Image< TPixel, VImageDimension >* inputImage );

  void InitializeLevelWindow();

  void EnableVolumeRendering(bool enable);

  void UpdateVolumeRenderingThreshold(int thValue);

};


#endif
