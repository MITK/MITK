/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNAxisGlyph.h"

#include <QColor>
#include <QFile>
#include <QImage>
#include <QPixmap>

namespace
{
  QString ResourcePath(QmitkMxNAxisGlyph glyph)
  {
    switch (glyph)
    {
      case QmitkMxNAxisGlyph::Pan:         return QStringLiteral(":/Qmitk/mxn-axis-pan.svg");
      case QmitkMxNAxisGlyph::Zoom:        return QStringLiteral(":/Qmitk/mxn-axis-zoom.svg");
      case QmitkMxNAxisGlyph::Slice:       return QStringLiteral(":/Qmitk/mxn-axis-slice.svg");
      case QmitkMxNAxisGlyph::Crosshair:   return QStringLiteral(":/Qmitk/mxn-axis-crosshair.svg");
      case QmitkMxNAxisGlyph::Orientation: return QStringLiteral(":/Qmitk/mxn-axis-orientation.svg");
      case QmitkMxNAxisGlyph::Windowing:   return QStringLiteral(":/Qmitk/mxn-axis-windowing.svg");
      case QmitkMxNAxisGlyph::Lut:         return QStringLiteral(":/Qmitk/mxn-axis-lut.svg");
      case QmitkMxNAxisGlyph::Selection:   return QStringLiteral(":/Qmitk/mxn-axis-selection.svg");
    }
    return QString();
  }
}

QPixmap QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph glyph, const QColor& color, int sizePx)
{
  QFile file(ResourcePath(glyph));
  if (!file.open(QIODevice::ReadOnly))
  {
    return QPixmap();
  }

  // Swap the placeholder color for the requested one, the same recolor trick
  // QmitkIconTheme uses for theme icons - here the color is a group hue (or
  // a grayed decoupled state) rather than the theme's icon color.
  QString svg = QString::fromUtf8(file.readAll());
  svg.replace(QStringLiteral("#00ff00"), color.name(QColor::HexRgb), Qt::CaseInsensitive);

  const QImage image = QImage::fromData(svg.toUtf8(), "svg");
  if (image.isNull())
  {
    return QPixmap();
  }

  return QPixmap::fromImage(image.scaled(sizePx, sizePx, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
