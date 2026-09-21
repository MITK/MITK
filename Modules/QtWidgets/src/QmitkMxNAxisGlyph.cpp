/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNAxisGlyph.h"

#include <QBuffer>
#include <QColor>
#include <QFile>
#include <QImage>
#include <QImageReader>
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

  // Rasterise at the requested size rather than at the resource's own 24 px and
  // scaling that up: the reader hands the size to the SVG renderer, so a glyph
  // stays crisp wherever it is drawn large (a peek plate's pumped axis is nearly
  // three times the resource size, where an upscale reads as a blurred bitmap).
  QByteArray data = svg.toUtf8();
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  QImageReader reader(&buffer, "svg");
  reader.setScaledSize(QSize(sizePx, sizePx));

  const QImage image = reader.read();
  if (image.isNull())
  {
    return QPixmap();
  }

  return QPixmap::fromImage(image);
}
