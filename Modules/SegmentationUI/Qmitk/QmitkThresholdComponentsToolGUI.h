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
#ifndef QMITK_QmitkThresholdComponentsToolGUI_H
#define QMITK_QmitkThresholdComponentsToolGUI_H

#include "itkImage.h"
#include "mitkDataStorage.h"
#include "mitkGeometry3D.h"
#include "mitkPointSet.h"

#include "qwidget.h"
#include "ui_QmitkThresholdComponentsToolGUIControls.h"

#include <MitkSegmentationUIExports.h>

#include "QmitkToolGUI.h"

#include "mitkThresholdComponentsTool.h"

class QmitkStdMultiWidget;
class DataNode;
class QmitkThresholdComponentsToolGUIControls;

/*!
*
* \brief QmitkThresholdComponentsToolGUI
*
* Threshold Components View class of the segmentation.
*
*/

class MITKSEGMENTATIONUI_EXPORT QmitkThresholdComponentsToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  /**
   * @brief mitkClassMacro
   */
  mitkClassMacro(QmitkThresholdComponentsToolGUI, QmitkToolGUI);

  itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    QmitkThresholdComponentsToolGUI(QWidget *parent = 0);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is
   * needed*/
  virtual void CreateConnections();

  ///** \brief Method to set the default data storage.*/
  virtual void SetDataStorage(mitk::DataStorage *dataStorage);

  /**
   * @brief Method to set the used multiwidget.
   * @param multiWidget
   */
  void SetMultiWidget(QmitkStdMultiWidget *multiWidget);

  /**
   * @brief Method to set the name of a data node.
   * @param labledSegmentation Name of the labeled segmentation
   * @param binaryImage Name of the binary image
   * @param surface Name of the surface
   */
  void SetDataNodeNames(std::string labledSegmentation,
                        std::string binaryImage,
                        /*std::string vesselTree,*/ std::string surface,
                        std::string maskedSegmentation);

  /**
   * @brief Method to enable/disable controls
   *
   * This method checks if a seed point is set and a segmentation exists.
   * @param enable/disable controls
   */
  void EnableControls(bool enable);

  /**
   * @brief Method to set the input image node
   * @param data node
   */
  void SetInputImageNode(mitk::DataNode *node);

  void Deactivated();
  void Activated();

  /**
  * @brief The created GUI from the .ui-File. This Attribute is obligatory
  */
  Ui::QmitkThresholdComponentsToolGUIControls m_Controls;

protected slots:

  /**
   * @brief Method to start the segmentation
   *
   * This method is called, when the "Start Segmentation" button is clicked.
   */
  void RunSegmentation();

  /**
   * @brief Method to confirm the preview segmentation
   *
   * This method is called, when the "Confirm Segmentation" button is clicked.
   */
  void ConfirmSegmentation();

  /**
   * @brief Method to switch the volume rendering on/off
   * @param on/off
   */
  void UseVolumeRendering(bool on);

  /**
   * @brief Method to set the lower threshold
   *
   * This method is called, when the minimum threshold slider has changed
   * @param lower threshold
   */
  void SetLowerThresholdValue(double lowerThreshold);

  /**
   * @brief Method to set upper threshold
   *
   * This Method is called, when the maximum threshold slider has changed
   * @param upper threshold
   */
  void SetUpperThresholdValue(double upperThreshold);

  /**
   * @brief Method to determine which tool to activate
   *
   * This method listens to the tool manager and activates this tool if requested otherwise disables this view
   */
  void OnNewToolAssociated(mitk::Tool *);

protected:
  mitk::ThresholdComponentsTool::Pointer m_ThresholdComponentsTool;

  /** \brief Destructor. */
  virtual ~QmitkThresholdComponentsToolGUI();

  // Pointer to the main widget to be able to reach the renderer
  QmitkStdMultiWidget *m_MultiWidget;

  mitk::DataStorage *m_DataStorage;

  mitk::DataNode::Pointer m_InputImageNode;

  /**
   * @brief Method to calculate parameter settings, when a seed point is set
   */
  void OnPointAdded();

private:
  std::string m_NAMEFORORGIMAGE;
  std::string m_NAMEFORLABLEDSEGMENTATIONIMAGE;
  std::string m_NAMEFORBINARYIMAGE;
  std::string m_NAMEFORSURFACE;
  std::string m_NAMEFORMASKEDSEGMENTATION;

  mitk::ScalarType m_LOWERTHRESHOLD; // Hounsfield value
  mitk::ScalarType m_UPPERTHRESHOLD; // Hounsfield value
  mitk::ScalarType m_SeedPointValueMean;
  mitk::ScalarType m_SeedPointValueVar;

  void RemoveHelperNodes();



  int m_SeedpointValue;
  bool m_UseVolumeRendering;
  bool m_SeedInitialized;

  long m_PointSetAddObserverTag;
  long m_PointSetMoveObserverTag;

  template <typename TPixel, unsigned int VImageDimension>
  void StartSegmentation(itk::Image<TPixel, VImageDimension> *itkImage,
                          mitk::BaseGeometry *imageGeometry,
                          mitk::PointSet *seedPointSet);


  void EnableVolumeRendering(bool enable);

};

#endif
