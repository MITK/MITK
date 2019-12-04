/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKUSCONTROLSBMODEWIDGET_H
#define QMITKUSCONTROLSBMODEWIDGET_H

#include <QWidget>

#include "MitkUSUIExports.h"
#include "mitkUSControlInterfaceBMode.h"

namespace Ui {
class QmitkUSControlsBModeWidget;
}

/**
  * \brief Widget for b mode controls of ultrasound devices.
  * This class handles the mitk::USControlInterfaceBMode of mitk::USDevice
  * objects.
  */
class MITKUSUI_EXPORT QmitkUSControlsBModeWidget : public QWidget
{
    Q_OBJECT

private slots:
  /**
    * \brief Called when user changes frequency value.
    */
  void OnFrequencyControlIndexChanged(int);

  /**
    * \brief Called when user changes power value.
    */
  void OnPowerControlValueChanged(int);

  /**
    * \brief Called when user changes depth value.
    */
  void OnDepthControlActivated(int);

  /**
    * \brief Called when user changes gain value.
    */
  void OnGainControlValueChanged(int);

  /**
    * \brief Called when user changes rejection value.
    */
  void OnRejectionControlValueChanged(int);

  /**
    * \brief Called when user changes dynamic range value.
    */
  void OnDynamicRangeControlValueChanged(int);

public:
  /**
    * A pointer to a concrete mitk::USControlInterfaceBMode is needed to
    * construct a QmitkUSControlsBModeWidget. Widget is ready after
    * constructing; slots are connected to gui controls.
    *
    * If a null pointer is given for 'controlInterface' all gui control elements
    * will be disabled.
    */
  explicit QmitkUSControlsBModeWidget(mitk::USControlInterfaceBMode::Pointer controlInterface, QWidget *parent = nullptr);
  ~QmitkUSControlsBModeWidget() override;

private:
  Ui::QmitkUSControlsBModeWidget*         ui;

  mitk::USControlInterfaceBMode::Pointer  m_ControlInterface;
};

#endif // QMITKUSCONTROLSBMODEWIDGET_H
