/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTrackingDeviceWidget_h
#define QmitkTrackingDeviceWidget_h

#include <QWidget>
#include "MitkIGTUIExports.h"

class MITKIGTUI_EXPORT QmitkTrackingDeviceWidget : public QWidget //MITKIGTUI_EXPORT
{
  Q_OBJECT

  public:
    QmitkTrackingDeviceWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkTrackingDeviceWidget() override;


};
#endif
