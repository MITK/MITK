/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUSControlsProbesWidget_h
#define QmitkUSControlsProbesWidget_h

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
  ~QmitkUSControlsProbesWidget() override;

private:
  Ui::QmitkUSControlsProbesWidget*         ui;

  mitk::USControlInterfaceProbes::Pointer  m_ControlInterface;
};

#endif
