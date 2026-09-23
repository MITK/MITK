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
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPixmapCache>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>

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

  // The outer ring meets the plate, whose fill over an image ranges from near
  // black to mid grey, and the neighbouring hues; white stands apart from all
  // of them. The inner ring keeps a light hue (yellow, light green) from
  // running into that white.
  const QColor StickerOuterInk(255, 255, 255);
  const QColor StickerInnerInk(7, 9, 11);

  qreal StickerInnerWidth(int sizePx)
  {
    return std::max(1.0, 0.03 * sizePx);
  }

  qreal StickerOuterWidth(int sizePx)
  {
    return std::max(1.0, 0.025 * sizePx);
  }

  /**
   * \brief The alpha of 'source', placed at 'offset' in an image of 'size',
   *        grown by a disc of 'radius' pixels.
   *
   * A disc rather than a few shifted copies: shifting in eight directions
   * leaves notches on the diagonals once the radius exceeds about two pixels.
   * Each source pixel stamps its alpha. A pixel whose centre lies 'd' away
   * spans the band from d - 1 to d past the glyph's edge and takes the part of
   * that band inside 'radius', which keeps a one-pixel ring opaque and the
   * ring's outer edge antialiased.
   */
  QImage DilatedAlpha(const QImage& source, const QSize& size, const QPoint& offset, qreal radius)
  {
    QImage mask(size, QImage::Format_Alpha8);
    mask.fill(0);
    const int reach = static_cast<int>(std::ceil(radius + 1.0));
    for (int sy = 0; sy < source.height(); ++sy)
    {
      const auto* sourceLine = reinterpret_cast<const QRgb*>(source.constScanLine(sy));
      for (int sx = 0; sx < source.width(); ++sx)
      {
        const int alpha = qAlpha(sourceLine[sx]);
        if (0 == alpha)
        {
          continue;
        }
        const int cx = sx + offset.x();
        const int cy = sy + offset.y();
        for (int y = std::max(0, cy - reach); y <= std::min(size.height() - 1, cy + reach); ++y)
        {
          uchar* line = mask.scanLine(y);
          for (int x = std::max(0, cx - reach); x <= std::min(size.width() - 1, cx + reach); ++x)
          {
            const qreal coverage = std::clamp(radius + 1.0 - std::hypot(x - cx, y - cy), 0.0, 1.0);
            line[x] = static_cast<uchar>(std::max<int>(line[x], qRound(alpha * coverage)));
          }
        }
      }
    }
    return mask;
  }

  QImage FilledWithAlpha(const QImage& alpha, const QColor& color)
  {
    QImage layer(alpha.size(), QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < alpha.height(); ++y)
    {
      const uchar* alphaLine = alpha.constScanLine(y);
      auto* line = reinterpret_cast<QRgb*>(layer.scanLine(y));
      for (int x = 0; x < alpha.width(); ++x)
      {
        const int a = alphaLine[x];
        line[x] = qRgba(color.red() * a / 255, color.green() * a / 255, color.blue() * a / 255, a);
      }
    }
    return layer;
  }

  /** \brief Rasterise 'glyph' in 'color' at 'sizePx'. With 'shapeOnly', every
   *         gradient stop is drawn opaque, so the image covers the glyph's
   *         full outline even where its paint fades out. */
  QImage RasterizeGlyph(QmitkMxNAxisGlyph glyph, const QColor& color, int sizePx, bool shapeOnly)
  {
    QFile file(ResourcePath(glyph));
    if (!file.open(QIODevice::ReadOnly))
    {
      return QImage();
    }

    // Swap the placeholder color for the requested one, the same recolor trick
    // QmitkIconTheme uses for theme icons - here the color is a group hue (or
    // a grayed decoupled state) rather than the theme's icon color.
    QString svg = QString::fromUtf8(file.readAll());
    svg.replace(QStringLiteral("#00ff00"), color.name(QColor::HexRgb), Qt::CaseInsensitive);
    if (shapeOnly)
    {
      static const QRegularExpression stopOpacity(QStringLiteral("stop-opacity=\"[^\"]*\""));
      svg.replace(stopOpacity, QStringLiteral("stop-opacity=\"1\""));
    }

    // Rasterise at the requested size rather than at the resource's own 24 px
    // and scaling that up: the reader hands the size to the SVG renderer, so a
    // glyph stays crisp wherever it is drawn large (a peek plate's pumped axis is
    // nearly three times the resource size, where an upscale reads as a blurred
    // bitmap).
    QByteArray data = svg.toUtf8();
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, "svg");
    reader.setScaledSize(QSize(sizePx, sizePx));
    return reader.read();
  }
}

QPixmap QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph glyph, const QColor& color, int sizePx)
{
  // The recolor drops alpha, so the RGB alone identifies the artwork.
  const QString key = QStringLiteral("QmitkMxNAxisGlyph/%1/%2/%3")
                        .arg(static_cast<int>(glyph))
                        .arg(color.name(QColor::HexRgb))
                        .arg(sizePx);
  QPixmap cached;
  if (QPixmapCache::find(key, &cached))
  {
    return cached;
  }

  const QImage image = RasterizeGlyph(glyph, color, sizePx, false);
  if (image.isNull())
  {
    return QPixmap();
  }

  const QPixmap pixmap = QPixmap::fromImage(image);
  QPixmapCache::insert(key, pixmap);
  return pixmap;
}

int QmitkMxNAxisGlyphStickerMargin(int sizePx)
{
  return static_cast<int>(std::ceil(StickerInnerWidth(sizePx) + StickerOuterWidth(sizePx)));
}

QPixmap QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph glyph, const QColor& color, int sizePx,
                                       qreal devicePixelRatio)
{
  const QString key = QStringLiteral("QmitkMxNAxisGlyphSticker/%1/%2/%3/%4")
                        .arg(static_cast<int>(glyph))
                        .arg(color.name(QColor::HexRgb))
                        .arg(sizePx)
                        .arg(devicePixelRatio);
  QPixmap cached;
  if (QPixmapCache::find(key, &cached))
  {
    return cached;
  }

  const QPixmap glyphPixmap = QmitkMxNRenderAxisGlyph(glyph, color, sizePx);
  if (glyphPixmap.isNull())
  {
    return QPixmap();
  }

  const qreal inner = StickerInnerWidth(sizePx);
  const qreal outer = StickerOuterWidth(sizePx);
  const int margin = QmitkMxNAxisGlyphStickerMargin(sizePx);
  // The rings follow the glyph's shape, not its paint: a glyph that fades out
  // (the lookup table's gradient) still gets a crisp outline all round.
  const QImage silhouette =
    RasterizeGlyph(glyph, color, sizePx, true).convertToFormat(QImage::Format_ARGB32);
  const QSize size(sizePx + 2 * margin, sizePx + 2 * margin);
  const QPoint offset(margin, margin);

  // The rings grow into every gap the glyph leaves, enclosed or not. Narrow
  // openings (the crosshair's centre) close, but an enclosed hole left open
  // would show a neighbouring glyph through the one lifted over it.
  QImage composite(size, QImage::Format_ARGB32_Premultiplied);
  composite.fill(Qt::transparent);
  {
    QPainter painter(&composite);
    painter.drawImage(0, 0, FilledWithAlpha(DilatedAlpha(silhouette, size, offset, inner + outer),
                                            StickerOuterInk));
    painter.drawImage(0, 0, FilledWithAlpha(DilatedAlpha(silhouette, size, offset, inner),
                                            StickerInnerInk));
    painter.drawImage(offset, glyphPixmap.toImage());
  }

  QPixmap pixmap = QPixmap::fromImage(composite);
  pixmap.setDevicePixelRatio(devicePixelRatio);
  QPixmapCache::insert(key, pixmap);
  return pixmap;
}

void QmitkMxNPaintStickerText(QPainter& painter, const QPointF& baseline, const QFont& font,
                              const QString& text, const QColor& color)
{
  QPainterPath path;
  path.addText(baseline, font, text);

  // Each ring is one logical pixel wide. A stroke is centred on the outline, so
  // it must be twice as wide as what it should show outside the text; the fill
  // drawn last covers the inner half.
  constexpr qreal ring = 1.0;
  painter.save();
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(StickerOuterInk, 2 * (ring + ring), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.drawPath(path);
  painter.setPen(QPen(StickerInnerInk, 2 * ring, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.drawPath(path);
  painter.setPen(Qt::NoPen);
  painter.setBrush(color);
  painter.drawPath(path);
  painter.restore();
}
