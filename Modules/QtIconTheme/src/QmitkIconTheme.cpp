/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkIconTheme.h>

#include <mitkLog.h>

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

  /* SVG allows both the six-digit and the equivalent three-digit notation of a
   * hexadecimal color code, hence both spellings of a magic color must be
   * replaced. The negative lookahead keeps the three-digit notation from
   * matching the leading half of an unrelated six-digit color code.
   */
  QString ReplaceMagicColor(const QString &svg, const QString &magicColors, const QString &themeColor)
  {
    const QRegularExpression re(QString("#(?:%1)(?![0-9a-f])").arg(magicColors),
      QRegularExpression::CaseInsensitiveOption);

    return QString(svg).replace(re, themeColor);
  }
}

QIcon QmitkIconTheme::GetIcon(const QByteArray &originalSVG)
{
  auto styleSheet = qApp->styleSheet();

  if (styleSheet.isEmpty())
    return QPixmap::fromImage(QImage::fromData(originalSVG));

  auto themedSVG = ReplaceColor(QString(originalSVG), GetColor());
  themedSVG = ReplaceMagicColor(themedSVG, QStringLiteral("ff00ff|f0f"), GetAccentColor());

  return QPixmap::fromImage(QImage::fromData(themedSVG.toLatin1()));
}

QIcon QmitkIconTheme::GetIcon(const QString &resourcePath)
{
  QFile resourceFile(resourcePath);

  if (resourceFile.open(QIODevice::ReadOnly))
  {
    auto originalSVG = resourceFile.readAll();
    return GetIcon(originalSVG);
  }

  MITK_WARN << "Could not read " << resourcePath.toStdString();
  return QIcon();
}

QString QmitkIconTheme::ReplaceColor(const QString &svg, const QString &color)
{
  return ReplaceMagicColor(svg, QStringLiteral("00ff00|0f0"), color);
}

QString QmitkIconTheme::GetColor()
{
  const auto styleSheet = qApp->styleSheet();
  const auto fallback = QStringLiteral("#000000");

  return !styleSheet.isEmpty()
    ? ParseColor(styleSheet, QStringLiteral("iconColor"), fallback)
    : fallback;
}

QString QmitkIconTheme::GetAccentColor()
{
  const auto styleSheet = qApp->styleSheet();
  const auto fallback = QStringLiteral("#ffffff");

  return !styleSheet.isEmpty()
    ? ParseColor(styleSheet, QStringLiteral("iconAccentColor"), fallback)
    : fallback;
}
