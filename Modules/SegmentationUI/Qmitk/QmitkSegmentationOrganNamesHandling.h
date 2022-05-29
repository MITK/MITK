/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationOrganNamesHandling_h
#define QmitkSegmentationOrganNamesHandling_h

#include <mitkColorProperty.h>

#include <MitkSegmentationUIExports.h>

#include <QColor>
#include <QString>
#include <QStringList>

namespace mitk
{
  namespace OrganNamesHandling
  {
    MITKSEGMENTATIONUI_EXPORT QStringList GetDefaultOrganColorString();
    MITKSEGMENTATIONUI_EXPORT void UpdateOrganList(QStringList& organColors, const QString& organname, Color color);
    MITKSEGMENTATIONUI_EXPORT void AppendToOrganList(QStringList& organColors, const QString& organname, const QColor& color);
  };
}

#endif
