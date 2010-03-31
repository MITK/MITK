/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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