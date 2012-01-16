/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-07-15 08:05:19 +0200 (Mi, 15 Jul 2009) $
Version:   $Revision: 11185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef QMITK_AdaptiveRegionGrowingView_H
#define QMITK_AdaptiveRegionGrowingView_H


#include "mitkDataStorage.h"
#include "itkImage.h"
#include "mitkGeometry3D.h"
#include "mitkPointSet.h"
//#include "mitkVesselSegmentationUIExports.h"

#include "qwidget.h"
#include "ui_QmitkAdaptiveRegionGrowingViewControls.h"

class QmitkStdMultiWidget;
class DataNode;
class QmitkAdaptiveRegionGrowingViewControls;


/*!
*
* \brief QmitkAdaptiveRegionGrowingView
*
* Adaptive Region Growing View class of the segmentation part of the navi-Broncho-Suite.
* User can choose between automatic or manual tracheobronchial tree and blood vessel segmentation
*
*/

class /*mitkVesselSegmentationUI_EXPORT*/ QmitkAdaptiveRegionGrowingView : public QWidget
{

  Q_OBJECT

public:
  typedef QmitkAdaptiveRegionGrowingView   Self;

  /**
  * @brief Constructor.
  **/
  QmitkAdaptiveRegionGrowingView(QWidget *parent=0);

  /** \brief Destructor. */
  virtual ~QmitkAdaptiveRegionGrowingView();

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();

  ///** \brief Method to set the default data storage.*/
  virtual void SetDataStorage(mitk::DataStorage* dataStorage)
  {
    m_DataStorage = dataStorage;
  }

  void SetMultiWidget(QmitkStdMultiWidget* multiWidget)
  {
    m_MultiWidget = multiWidget;
  }

  void SetDataNodeNames(std::string labledSegmentation, std::string binaryImage, /*std::string vesselTree,*/ std::string surface, std::string seedPoint);

  void EnableControls(bool enable);

  void SetInputImageNode(mitk::DataNode* node)
  {
    m_InputImageNode = node;
  }

  //void SetOrganType(int type);
  
  ///** \brief Defines fixed threshold for region growing (true = LOWERTHRESHOLD fixed, false = UPPERTHRESHOLD fixed)
  void SetRegionGrowingDirectionUpwards(bool upwards)
  {
    m_CurrentRGDirectionIsUpwards=upwards;
  }

  void Deactivated();
  void Activated();


  /**
  * @brief The created GUI from the .ui-File. This Attribute is obligatory
  */
  Ui::QmitkAdaptiveRegionGrowingViewControls m_Controls;


 // itkSetMacro(labledSegmentation,std::string);
//  void ChangeBronchialVesselView(int typeID); // switch from bronchial segmentaion view to blood vessel segmentation view ...

signals:
//  void EnableBoostAndCleaningWidget(bool on);
//  void EntireVesselTreeDeleted(bool vesselTreeDeleted);

  protected slots:
    void SetSeedPointToggled(bool toggled);
    void RunSegmentation();
    void ChangeLevelWindow(int newValue);//called, when the Level Window is changed via the slider in the ControlWidget
    void IncreaseSlider();//called, when the slider-position is modified via the +/- buttons
    void DecreaseSlider();
    void ConfirmSegmentation();
    void UseVolumeRendering(bool on);
//    void SetAutomaticSeedPointDetection();
    void SetLowerThresholdValue(int lowerThreshold);

    void SetUpperThresholdValue(int upperThreshold);



    

protected:

  //Pointer to the main widget to be able to reach the renderer
  QmitkStdMultiWidget* m_MultiWidget;

  mitk::DataStorage* m_DataStorage;

  mitk::DataNode::Pointer m_InputImageNode;


  void DeactivateSeedPointMode();
  void ActivateSeedPointMode();

  void OnPointAdded();

private:

  std::string m_NAMEFORORGIMAGE;
  std::string m_NAMEFORSEEDPOINT;
  std::string m_NAMEFORLABLEDSEGMENTATIONIMAGE;
  std::string m_NAMEFORBINARYIMAGE;
  std::string m_NAMEFORVESSELTREE;
  std::string m_NAMEFORSURFACE;

  int m_OrganType;
  mitk::ScalarType m_LOWERTHRESHOLD; //Hounsfield value
  mitk::ScalarType m_UPPERTHRESHOLD; //Hounsfield value
  mitk::ScalarType m_SeedPointValueMean;
  bool m_CurrentRGDirectionIsUpwards; // defines fixed threshold (true = LOWERTHRESHOLD fixed, false = UPPERTHRESHOLD fixed)

  int m_SeedpointValue;
  int m_DetectedLeakagePoint;
  bool m_SliderInitialized;
  bool m_UseVolumeRendering;
  bool m_UpdateSuggestedThreshold;
  float m_SuggestedThValue;

  long m_PointSetAddObserverTag;

  template < typename TPixel, unsigned int VImageDimension >
  void StartRegionGrowing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry, mitk::PointSet::PointType seedPoint );

  void InitializeLevelWindow();

  void EnableVolumeRendering(bool enable);

  void UpdateVolumeRenderingThreshold(int thValue);
  
//  void DefaultSettingsEnable(bool val);
  
  
};


#endif
