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

#ifndef QMITKUSCONTROLSBMODEWIDGET_H
#define QMITKUSCONTROLSBMODEWIDGET_H

#include <QWidget>

#include "MitkUSUIExports.h"
#include "mitkUSControlInterfaceBMode.h"

namespace Ui {
class QmitkUSControlsBModeWidget;
}

class MitkUSUI_EXPORT QmitkUSControlsBModeWidget : public QWidget
{
    Q_OBJECT

private slots:
  void OnDepthControlActivated(int);
  void OnGainControlValueChanged(int);
  void OnRejectionControlValueChanged(int);

public:
  explicit QmitkUSControlsBModeWidget(mitk::USControlInterfaceBMode::Pointer controlInterface, QWidget *parent = 0);
  ~QmitkUSControlsBModeWidget();

private:
  Ui::QmitkUSControlsBModeWidget*         ui;

  mitk::USControlInterfaceBMode::Pointer  m_ControlInterface;
};

#endif // QMITKUSCONTROLSBMODEWIDGET_H
