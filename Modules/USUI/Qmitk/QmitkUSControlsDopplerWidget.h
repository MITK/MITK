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

#ifndef QMITKUSCONTROLSDOPPLERWIDGET_H
#define QMITKUSCONTROLSDOPPLERWIDGET_H

#include <QWidget>

#include "MitkUSUIExports.h"
#include "mitkUSControlInterfaceDoppler.h"

namespace Ui {
class QmitkUSControlsDopplerWidget;
}

/**
  * \brief Widget for b mode controls of ultrasound devices.
  * This class handles the mitk::USControlInterfaceDoppler of mitk::USDevice
  * objects.
  */
class MITKUSUI_EXPORT QmitkUSControlsDopplerWidget : public QWidget
{
    Q_OBJECT

private slots:

public:
  /**
    * A pointer to a concrete mitk::USControlInterfaceDoppler is needed to
    * construct a QmitkUSControlsBModeWidget. Widget is ready after
    * constructing; slots are connected to gui controls.
    *
    * If a null pointer is given for 'controlInterface' all gui control elements
    * will be disabled.
    */
  explicit QmitkUSControlsDopplerWidget(mitk::USControlInterfaceDoppler::Pointer controlInterface, QWidget *parent = nullptr);
  ~QmitkUSControlsDopplerWidget();

private:
  Ui::QmitkUSControlsDopplerWidget*         ui;

  mitk::USControlInterfaceDoppler::Pointer  m_ControlInterface;
};

#endif // QMITKUSCONTROLSDOPPLERWIDGET_H