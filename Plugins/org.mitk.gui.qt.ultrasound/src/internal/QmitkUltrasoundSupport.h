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

#ifndef QmitkUltrasoundSupport_h
#define QmitkUltrasoundSupport_h

#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkUltrasoundSupportControls.h"
#include "QmitkUSAbstractCustomWidget.h"
#include "QmitkUSControlsBModeWidget.h"
#include "QmitkUSControlsDopplerWidget.h"
#include "QmitkUSControlsProbesWidget.h"
#include <mitkBaseRenderer.h>
#include "QmitkRenderWindow.h"
#include <mitkStandaloneDataStorage.h>
#include <QmitkLevelWindowWidget.h>
#include <QmitkSliceWidget.h>

#include <QTime>

#include <ctkServiceEvent.h>

/*!
\brief UltrasoundSupport
This plugin provides functionality to manage Ultrasound devices, create video devices and to view device images.

\ingroup ${plugin_target}_internal
*/
class QmitkUltrasoundSupport : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  virtual void SetFocus() override;

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent) override;

  QmitkUltrasoundSupport();
  virtual ~QmitkUltrasoundSupport();

  public slots:
  /*
  * \brief This is called when the newDeviceWidget is closed
  */
  void OnNewDeviceWidgetDone();

  protected slots:

  void OnClickedAddNewDevice();

  void OnChangedFramerateLimit();

  void OnClickedEditDevice();

  /*
   *\brief Called, when the selection in the list of the active devices changes.
   */
  void OnChangedActiveDevice();

  void OnClickedFreezeButton();

  void OnDeviceServiceEvent(const ctkServiceEvent event);

  /*
  * \brief This is the main imaging loop that updates the image and is called regularily during the imaging process
  */
  void UpdateImage();

  void RenderImage2d();

  void RenderImage3d();

  void StartTimers();

  void StopTimers();

protected:

  void CreateControlWidgets();
  void RemoveControlWidgets();

  Ui::UltrasoundSupportControls* m_Controls;

  QmitkUSAbstractCustomWidget*  m_ControlCustomWidget;
  QmitkUSControlsBModeWidget*   m_ControlBModeWidget;
  QmitkUSControlsDopplerWidget* m_ControlDopplerWidget;
  QmitkUSControlsProbesWidget*  m_ControlProbesWidget;

  bool m_ImageAlreadySetToNode;

  unsigned int m_CurrentImageWidth;
  unsigned int m_CurrentImageHeight;

  /** Keeps track of the amount of output Nodes*/
  unsigned int m_AmountOfOutputs;

  /** The device that is currently used to aquire images */
  mitk::USDevice::Pointer m_Device;

  void SetTimerIntervals(int intervalPipeline, int interval2D, int interval3D);
  /** This timer triggers periodic updates to the pipeline */
  QTimer* m_UpdateTimer;
  QTimer* m_RenderingTimer2d;
  QTimer* m_RenderingTimer3d;

  /** These clocks are used to compute the framerate in the methods DisplayImage(),RenderImage2d() and RenderImage3d(). */
  QTime  m_Clock;
  QTime  m_Clock2d;
  QTime  m_Clock3d;

  /** A counter to comute the framerate. */
  int m_FrameCounterPipeline;
  int m_FrameCounter2d;
  int m_FrameCounter3d;
  int m_FPSPipeline, m_FPS2d, m_FPS3d;

  /** Stores the properties of some QWidgets (and the tool storage file name) to QSettings.*/
  void StoreUISettings();

  /** Loads the properties of some QWidgets (and the tool storage file name) from QSettings.*/
  void LoadUISettings();

  /** The nodes that we feed images into.*/
  std::vector<mitk::DataNode::Pointer> m_Node;
  /** Adds a new node to the m_Nodes vector*/
  void InitNewNode();
  /** Destroys the last node in the m_Nodes vector */
  void DestroyLastNode();
  /** Checks the amount of slices in the image from the USDevice and creates as many Nodes as there are slices */
  void UpdateAmountOfOutputs();

  /** This function just checks how many nodes there are currently and sets the laser image to a jet transparent colormap. */
  void UpdateLevelWindows();
  bool m_ForceRequestUpdateAll;

  void SetColormap(mitk::DataNode::Pointer node, mitk::LookupTable::LookupTableType type);

  /** The seperated slices from m_Image */
  std::vector<mitk::Image::Pointer>   m_curOutput;

  /** The old geometry of m_Image. It is needed to check if the geometry changed (e.g. because
   *  the zoom factor was modified) and the image needs to be reinitialized. */
  mitk::SlicedGeometry3D::Pointer m_OldGeometry;

  QList<ctkServiceReference>    m_CustomWidgetServiceReference;

  double m_CurrentDynamicRange;

  /* Spacing calibration variables and methods */
  mitk::Point3D m_Xpoint1,m_Xpoint2,m_Ypoint1,m_Ypoint2;
  double m_XSpacing, m_YSpacing;
  double ComputeSpacing(mitk::Point3D p1, mitk::Point3D p2, double distance);
  protected slots:
  void SetXPoint1();
  void SetXPoint2();
  void SetYPoint1();
  void SetYPoint2();
  void WriteSpacingToDevice();
};

#endif // UltrasoundSupport_h
