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

class MitkUSUI_EXPORT QmitkUSControlsProbesWidget : public QWidget
{
    Q_OBJECT

private slots:
  void OnProbeControlActivated(int index);

public:
  explicit QmitkUSControlsProbesWidget(mitk::USControlInterfaceProbes::Pointer controlInterface, QWidget *parent = 0);
  ~QmitkUSControlsProbesWidget();

private:
  Ui::QmitkUSControlsProbesWidget*         ui;

  mitk::USControlInterfaceProbes::Pointer  m_ControlInterface;
};

#endif // QMITKUSCONTROLSPROBESWIDGET_H
