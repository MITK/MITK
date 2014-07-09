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

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class UltrasoundSupport : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  virtual void SetFocus();

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  UltrasoundSupport();
  virtual ~UltrasoundSupport();

  public slots:
    /*
    * \brief This is called when the newDeviceWidget is closed
    */
    void OnNewDeviceWidgetDone();

    protected slots:

      void OnClickedAddNewDevice();

      void OnChangedFramerateLimit(int);

      void OnChangedActiveDevice();

      void OnClickedFreezeButton();

      void OnDeciveServiceEvent(const ctkServiceEvent event);

      /*
       *\brief Called, when the selection in the list of the active devices changes.
       */
      void OnActiveDeviceSelectionChanged(us::ServiceReferenceU reference);

      /*
      * \brief This is the main imaging loop that is called regularily during the imaging process
      */
      void DisplayImage();

protected:

  void CreateControlWidgets();
  void RemoveControlWidgets();

  int m_FrameCounter;

  /*
  * \brief This timer triggers periodic updates to the pipeline
  */
  QTimer* m_Timer;

  QTime   m_Clock;

  /*
  * \brief The device that is currently used to aquire images
  */
  mitk::USDevice::Pointer m_Device;

  /*
  * \brief The node that we feed images into
  */
  mitk::DataNode::Pointer m_Node;

  mitk::Image::Pointer m_Image;

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
