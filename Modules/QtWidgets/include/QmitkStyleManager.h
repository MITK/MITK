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

#ifndef QmitkStyleManager_h
#define QmitkStyleManager_h

#include <MitkQtWidgetsExports.h>

#include <QByteArray>
#include <QIcon>
#include <QString>

class MITKQTWIDGETS_EXPORT QmitkStyleManager
{
public:
  static QIcon ThemeIcon(const QByteArray &originalSVG);
  static QIcon ThemeIcon(const QString &resourcePath);

  QmitkStyleManager() = delete;
  ~QmitkStyleManager() = delete;
};

#endif
