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

#ifndef QMITKTRACKINGDEVICEWIDGET_H
#define QMITKTRACKINGDEVICEWIDGET_H

#include <QWidget>
#include "MitkIGTUIExports.h"

class MitkIGTUI_EXPORT QmitkTrackingDeviceWidget : public QWidget //MitkIGTUI_EXPORT
{
  Q_OBJECT

  public:
    QmitkTrackingDeviceWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkTrackingDeviceWidget();


};
#endif
