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

#ifndef QMITKUSCONTROLSPROBESWIDGET_H
#define QMITKUSCONTROLSPROBESWIDGET_H

#include <QWidget>

#include "MitkUSUIExports.h"
#include "mitkUSControlInterfaceProbes.h"

namespace Ui {
class QmitkUSControlsProbesWidget;
}

/**
  * \brief Widget for probes controls of ultrasound devices.
  * This class handles the mitk::USControlInterfaceProbes of mitk::USDevice
  * objects.
  */
class MITKUSUI_EXPORT QmitkUSControlsProbesWidget : public QWidget
{
    Q_OBJECT

private slots:
  /**
    * \brief Called when user changes selected probe.
    */
  void OnProbeControlActivated(int index);

public:
  /**
    * A pointer to a concrete mitk::USControlInterfaceProbes is needed to
    * construct a QmitkUSControlsBModeWidget. Widget is ready after
    * constructing; slots are connected to gui controls.
    *
    * If a null pointer is given for 'controlInterface' all gui control elements
    * will be disabled.
    */
  explicit QmitkUSControlsProbesWidget(mitk::USControlInterfaceProbes::Pointer controlInterface, QWidget *parent = nullptr);
  ~QmitkUSControlsProbesWidget();

private:
  Ui::QmitkUSControlsProbesWidget*         ui;

  mitk::USControlInterfaceProbes::Pointer  m_ControlInterface;
};

#endif // QMITKUSCONTROLSPROBESWIDGET_H
