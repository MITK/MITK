/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkStyleManager.h>

#include <mitkLogMacros.h>

#include <QApplication>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QRegularExpression>

namespace
{
  QString ParseColor(const QString &subject, const QString &colorName, const QString &fallback)
  {
    const QString pattern = QString("%1\\s*[=:]\\s*(#[0-9a-f]{6})").arg(colorName);

    QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
    auto match = re.match(subject);

    return match.hasMatch()
      ? match.captured(1)
      : fallback;
  }
}

QIcon QmitkStyleManager::ThemeIcon(const QByteArray &originalSVG)
{
  auto styleSheet = qApp->styleSheet();

  if (styleSheet.isEmpty())
    return QPixmap::fromImage(QImage::fromData(originalSVG));

  auto iconColor = GetIconColor();
  auto iconAccentColor = GetIconAccentColor();

  auto themedSVG = QString(originalSVG).replace(QStringLiteral("#00ff00"), iconColor, Qt::CaseInsensitive);
  themedSVG = themedSVG.replace(QStringLiteral("#ff00ff"), iconAccentColor, Qt::CaseInsensitive);

  return QPixmap::fromImage(QImage::fromData(themedSVG.toLatin1()));
}

QIcon QmitkStyleManager::ThemeIcon(const QString &resourcePath)
{
  QFile resourceFile(resourcePath);

  if (resourceFile.open(QIODevice::ReadOnly))
  {
    auto originalSVG = resourceFile.readAll();
    return ThemeIcon(originalSVG);
  }

  MITK_WARN << "Could not read " << resourcePath.toStdString();
  return QIcon();
}

QString QmitkStyleManager::GetIconColor()
{
  const auto styleSheet = qApp->styleSheet();
  const auto fallback = QStringLiteral("#000000");

  return !styleSheet.isEmpty()
    ? ParseColor(styleSheet, QStringLiteral("iconColor"), fallback)
    : fallback;
}

QString QmitkStyleManager::GetIconAccentColor()
{
  const auto styleSheet = qApp->styleSheet();
  const auto fallback = QStringLiteral("#ffffff");

  return !styleSheet.isEmpty()
    ? ParseColor(styleSheet, QStringLiteral("iconAccentColor"), fallback)
    : fallback;
}
