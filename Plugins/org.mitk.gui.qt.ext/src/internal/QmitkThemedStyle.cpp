/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkThemedStyle.h"

#include <berryQtStyleManager.h>

#include <QFile>

namespace
{
  QString GetIconNameForStandardPixmap(QStyle::StandardPixmap sp)
  {
    switch (sp)
    {
      case QStyle::SP_DialogOkButton:
      case QStyle::SP_DialogYesButton:
        return "actions/dialog-ok";

      case QStyle::SP_DialogCancelButton:
      case QStyle::SP_DialogNoButton:
        return "actions/dialog-cancel";

      case QStyle::SP_MessageBoxCritical:
        return "status/dialog-error";

      case QStyle::SP_MessageBoxInformation:
        return "status/dialog-information";

      case QStyle::SP_MessageBoxQuestion:
        return "status/dialog-question";

      case QStyle::SP_MessageBoxWarning:
        return "status/dialog-warning";

      default:
        return {};
    }
  }
}

QIcon QmitkThemedStyle::standardIcon(StandardPixmap sp, const QStyleOption* opt, const QWidget* widget) const
{
  const auto name = GetIconNameForStandardPixmap(sp);

  if (!name.isEmpty())
  {
    const auto path = QString(":/org_mitk_icons/icons/awesome/scalable/%1.svg").arg(name);

    if (QFile::exists(path))
      return berry::QtStyleManager::ThemeIcon(path);
  }

  return QProxyStyle::standardIcon(sp, opt, widget);
}
