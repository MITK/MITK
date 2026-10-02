/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkThemedStyle.h"

#include <QmitkIconTheme.h>

#include <QFile>

namespace
{
  QString GetIconPathForStandardPixmap(QStyle::StandardPixmap sp)
  {
    const QString awesomePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/%1.svg");

    switch (sp)
    {
      case QStyle::SP_DialogOkButton:
      case QStyle::SP_DialogYesButton:
        return awesomePath.arg("actions/dialog-ok");

      case QStyle::SP_DialogCancelButton:
      case QStyle::SP_DialogNoButton:
        return awesomePath.arg("actions/dialog-cancel");

      case QStyle::SP_MessageBoxCritical:
        return awesomePath.arg("status/dialog-error");

      case QStyle::SP_MessageBoxInformation:
        return awesomePath.arg("status/dialog-information");

      case QStyle::SP_MessageBoxQuestion:
        return awesomePath.arg("status/dialog-question");

      case QStyle::SP_MessageBoxWarning:
        return awesomePath.arg("status/dialog-warning");

      // Some native styles draw these in the color of the system palette,
      // which the application themes do not change.
      case QStyle::SP_ToolBarHorizontalExtensionButton:
        return QStringLiteral(":/org.mitk.gui.qt.ext/toolbar_extension_horizontal.svg");

      case QStyle::SP_ToolBarVerticalExtensionButton:
        return QStringLiteral(":/org.mitk.gui.qt.ext/toolbar_extension_vertical.svg");

      default:
        return {};
    }
  }
}

QIcon QmitkThemedStyle::standardIcon(StandardPixmap sp, const QStyleOption* opt, const QWidget* widget) const
{
  const auto path = GetIconPathForStandardPixmap(sp);

  if (!path.isEmpty() && QFile::exists(path))
    return QmitkIconTheme::GetIcon(path);

  return QProxyStyle::standardIcon(sp, opt, widget);
}
