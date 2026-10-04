/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNAxisGlyph_h
#define QmitkMxNAxisGlyph_h

#include <MitkQtWidgetsExports.h>

#include <QtGlobal>

class QColor;
class QFont;
class QPainter;
class QPixmap;
class QPointF;
class QString;

/**
 * \brief The synchronization axes that carry a compact viewport glyph: the
 *        seven per-dimension axes plus data selection.
 *
 * This is a presentation enum, deliberately separate from
 * QmitkMxNSyncDimension (a load-bearing data enum that sizes the per-cell link
 * store): surfaces that only draw axis glyphs depend on this, not on the data
 * model, and selection - which has no QmitkMxNSyncDimension value - takes its
 * place here as an equal axis.
 */
enum class QmitkMxNAxisGlyph
{
  Pan,
  Zoom,
  Slice,
  Crosshair,
  Orientation,
  Windowing,
  Lut,
  Selection
};

/**
 * \brief Render an axis glyph, recolored to 'color', as a 'sizePx' square
 *        pixmap (device pixels) carrying 'devicePixelRatio'.
 *
 * The glyphs are embedded SVG resources whose placeholder color (`#00ff00`,
 * the same convention QmitkIconTheme uses) is swapped for 'color' at load,
 * so a glyph can take any foreground (a group hue, a grayed decoupled state)
 * and stay crisp at any size. Returns a null pixmap if the resource is missing.
 */
MITKQTWIDGETS_EXPORT QPixmap QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph glyph, const QColor& color, int sizePx,
                                                     qreal devicePixelRatio = 1.0);

/**
 * \brief Render an axis glyph as a sticker: the glyph in 'color' at 'sizePx'
 *        (device pixels), outlined by a dark inner and a white outer ring - a
 *        glyph lifted to the front of whatever it overlaps.
 *
 * The rings grow outward, so the pixmap is larger than the glyph by
 * QmitkMxNAxisGlyphStickerMargin(sizePx) on every side; draw it centred on
 * where the plain glyph would go. The pixmap already carries
 * 'devicePixelRatio'. Glyph and rings form one composite, so painting it at
 * reduced opacity fades the sticker as a whole rather than letting the rings
 * show through the glyph. Returns a null pixmap if the resource is missing.
 */
MITKQTWIDGETS_EXPORT QPixmap QmitkMxNRenderAxisGlyphSticker(QmitkMxNAxisGlyph glyph,
                                                            const QColor& color,
                                                            int sizePx,
                                                            qreal devicePixelRatio);

/** \brief The device pixels a sticker of 'sizePx' extends past its glyph on
 *         each side. */
MITKQTWIDGETS_EXPORT int QmitkMxNAxisGlyphStickerMargin(int sizePx);

/**
 * \brief Paint 'text' in 'color' from 'baseline' with the sticker's two rings,
 *        so a value lifted over its neighbours stays legible and reads as part
 *        of the sticker glyph it belongs to.
 */
MITKQTWIDGETS_EXPORT void QmitkMxNPaintStickerText(QPainter& painter,
                                                   const QPointF& baseline,
                                                   const QFont& font,
                                                   const QString& text,
                                                   const QColor& color);

#endif
