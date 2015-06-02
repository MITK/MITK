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

class MITKIGTUI_EXPORT QmitkTrackingDeviceWidget : public QWidget //MITKIGTUI_EXPORT
{
  Q_OBJECT

  public:
    QmitkTrackingDeviceWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkTrackingDeviceWidget();


};
#endif
