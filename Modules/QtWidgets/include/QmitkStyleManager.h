/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
