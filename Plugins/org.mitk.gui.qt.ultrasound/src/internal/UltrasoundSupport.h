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

#ifndef UltrasoundSupport_h
#define UltrasoundSupport_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_UltrasoundSupportControls.h"
#include "QmitkUSAbstractCustomWidget.h"
#include "QmitkUSControlsBModeWidget.h"
#include "QmitkUSControlsDopplerWidget.h"
#include "QmitkUSControlsProbesWidget.h"

#include <QTime>

#include <ctkServiceEvent.h>

/*!
\brief UltrasoundSupport
This plugin provides functionality to manage Ultrasound devices, create video devices and to view device images.

\ingroup ${plugin_target}_internal
*/
class UltrasoundSupport : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  virtual void SetFocus() override;

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent) override;

  UltrasoundSupport();
  virtual ~UltrasoundSupport();

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

  void OnDeciveServiceEvent(const ctkServiceEvent event);

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

  /** The node that we feed images into.*/
  mitk::DataNode::Pointer m_Node;

  /** The image that is hold by the node above.*/
  mitk::Image::Pointer m_Image;

  /** The old geometry of m_Image. It is needed to check if the geometry changed (e.g. because
   *  the zoom factor was modified) and the image needs to be reinitialized. */
  mitk::SlicedGeometry3D::Pointer m_OldGeometry;

  Ui::UltrasoundSupportControls m_Controls;

  QmitkUSAbstractCustomWidget*  m_ControlCustomWidget;
  QmitkUSControlsBModeWidget*   m_ControlBModeWidget;
  QmitkUSControlsDopplerWidget* m_ControlDopplerWidget;
  QmitkUSControlsProbesWidget*  m_ControlProbesWidget;

  QList<ctkServiceReference>    m_CustomWidgetServiceReference;

  bool m_ImageAlreadySetToNode;
  unsigned int m_CurrentImageWidth;
  unsigned int m_CurrentImageHeight;
  double m_CurrentDynamicRange;
};

#endif // UltrasoundSupport_h
