/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITK_QmitkAdaptiveRegionGrowingToolGUI_H
#define QMITK_QmitkAdaptiveRegionGrowingToolGUI_H

#include "itkImage.h"
#include "mitkDataStorage.h"
#include "mitkGeometry3D.h"
#include "mitkPointSet.h"

#include "qwidget.h"
#include "ui_QmitkAdaptiveRegionGrowingToolGUIControls.h"

#include <MitkSegmentationUIExports.h>

#include "QmitkToolGUI.h"

#include "mitkAdaptiveRegionGrowingTool.h"

class DataNode;
class QmitkAdaptiveRegionGrowingToolGUIControls;

/*!
*
* \brief QmitkAdaptiveRegionGrowingToolGUI
*
* Adaptive Region Growing View class of the segmentation.
*
*/

class MITKSEGMENTATIONUI_EXPORT QmitkAdaptiveRegionGrowingToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  /**
   * @brief mitkClassMacro
   */
  mitkClassMacro(QmitkAdaptiveRegionGrowingToolGUI, QmitkToolGUI);

  itkFactorylessNewMacro(Self);

  itkCloneMacro(Self);

    QmitkAdaptiveRegionGrowingToolGUI(QWidget *parent = nullptr);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is
   * needed*/
  virtual void CreateConnections();

  ///** \brief Method to set the default data storage.*/
  virtual void SetDataStorage(mitk::DataStorage *dataStorage);

  /**
   * @brief Method to set the name of a data node.
   * @param labledSegmentation Name of the labeled segmentation
   * @param binaryImage Name of the binary image
   * @param surface Name of the surface
   * @param maskedSegmentation
   */
  void SetDataNodeNames(std::string labledSegmentation,
                        std::string binaryImage,
                        /*std::string vesselTree,*/ std::string surface,
                        std::string maskedSegmentation);

  /**
   * @brief Method to enable/disable controls for region growing
   *
   * This method checks if a seed point is set and a segmentation exists.
   * @param enable enable/disable controls
   */
  void EnableControls(bool enable);

  /**
   * @brief Method to set the input image node
   * @param node data node
   */
  void SetInputImageNode(mitk::DataNode *node);

  void Deactivated();
  void Activated();

  /**
  * @brief The created GUI from the .ui-File. This Attribute is obligatory
  */
  Ui::QmitkAdaptiveRegionGrowingToolGUIControls m_Controls;

protected slots:

  /**
   * @brief Method to start the segmentation
   *
   * This method is called, when the "Start Segmentation" button is clicked.
   */
  void RunSegmentation();

  /**
   * @brief Method to change the level window
   *
   * This method is called, when the level window slider is changed via the slider in the control widget
   * @param newValue new value
   */
  void ChangeLevelWindow(double newValue);

  /**
   * @brief Method to increase the preview slider
   *
   * This method is called, when the + button is clicked and increases the value by 1
   */
  void IncreaseSlider();

  /**
   * @brief Method to decrease the preview slider
   *
   * This method is called, when the - button is clicked and decreases the value by 1
   */
  void DecreaseSlider();

  /**
   * @brief Method to confirm the preview segmentation
   *
   * This method is called, when the "Confirm Segmentation" button is clicked.
   */
  void ConfirmSegmentation();

  /**
   * @brief Method to switch the volume rendering on/off
   * @param on
   */
  void UseVolumeRendering(bool on);

  /**
   * @brief Method to set the lower threshold
   *
   * This method is called, when the minimum threshold slider has changed
   * @param lowerThreshold lower threshold
   */
  void SetLowerThresholdValue(double lowerThreshold);

  /**
   * @brief Method to set upper threshold
   *
   * This Method is called, when the maximum threshold slider has changed
   * @param upperThreshold upper threshold
   */
  void SetUpperThresholdValue(double upperThreshold);

  /**
   * @brief Method to determine which tool to activate
   *
   * This method listens to the tool manager and activates this tool if requested otherwise disables this view
   */
  void OnNewToolAssociated(mitk::Tool *);

protected:
  mitk::AdaptiveRegionGrowingTool::Pointer m_RegionGrow3DTool;

  /** \brief Destructor. */
  ~QmitkAdaptiveRegionGrowingToolGUI() override;

  mitk::DataStorage *m_DataStorage;

  mitk::DataNode::Pointer m_InputImageNode;

  /**
   * @brief Method to calculate parameter settings, when a seed point is set
   */
  void OnPointAdded();

   /**
   * @brief Method to extract a 3D image based on a given time point that can be taken from the SliceNavigationController
   *
   * This ensures that the seed point is taken from the current selected 3D image
   */
  mitk::Image::ConstPointer GetImageByTimePoint(const mitk::Image *image,
                                                          mitk::TimePointType timePoint) const;

private:
  std::string m_NAMEFORORGIMAGE;
  std::string m_NAMEFORLABLEDSEGMENTATIONIMAGE;
  std::string m_NAMEFORBINARYIMAGE;
  std::string m_NAMEFORSURFACE;
  std::string m_NAMEFORMASKEDSEGMENTATION;

  mitk::ScalarType m_LOWERTHRESHOLD; // Hounsfield value
  mitk::ScalarType m_UPPERTHRESHOLD; // Hounsfield value
  mitk::ScalarType m_SeedPointValueMean;

  void RemoveHelperNodes();

  int m_DetectedLeakagePoint;

  bool m_CurrentRGDirectionIsUpwards; // defines fixed threshold (true = LOWERTHRESHOLD fixed, false = UPPERTHRESHOLD
                                      // fixed)

  int m_SeedpointValue;
  bool m_SliderInitialized;
  bool m_UseVolumeRendering;
  bool m_UpdateSuggestedThreshold;
  float m_SuggestedThValue;

  long m_PointSetAddObserverTag;
  long m_PointSetMoveObserverTag;

  template <typename TPixel, unsigned int VImageDimension>
  void StartRegionGrowing(const itk::Image<TPixel, VImageDimension> *itkImage,
                          const mitk::BaseGeometry *imageGeometry,
                          const mitk::PointSet::PointType seedPoint);

  template <typename TPixel, unsigned int VImageDimension>
  void ITKThresholding(itk::Image<TPixel, VImageDimension> *inputImage);

  void InitializeLevelWindow();

  void EnableVolumeRendering(bool enable);

  void UpdateVolumeRenderingThreshold(int thValue);
};

#endif
