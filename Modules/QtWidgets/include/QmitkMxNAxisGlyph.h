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

#include <QmitkMxNSyncDimension.h>

#include <QColor>
#include <QtGlobal>

class QPixmap;

/**
 * \brief The synchronization axes that carry a compact viewport glyph: the
 *        seven per-dimension axes plus data selection and time.
 *
 * This is a presentation enum, deliberately separate from
 * QmitkMxNSyncDimension (a load-bearing data enum that sizes the per-cell link
 * store): surfaces that only draw axis glyphs depend on this, not on the data
 * model, and selection and time - which have no QmitkMxNSyncDimension value -
 * take their place here as equal axes.
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
  Selection,
  Time
};

/** \brief The glyph that presents a synchronization axis on every surface. */
inline constexpr QmitkMxNAxisGlyph QmitkMxNSyncAxisGlyph(QmitkMxNSyncAxis axis)
{
  switch (axis)
  {
    case QmitkMxNSyncAxis::Orientation: return QmitkMxNAxisGlyph::Orientation;
    case QmitkMxNSyncAxis::Pan:         return QmitkMxNAxisGlyph::Pan;
    case QmitkMxNSyncAxis::Zoom:        return QmitkMxNAxisGlyph::Zoom;
    case QmitkMxNSyncAxis::Slice:       return QmitkMxNAxisGlyph::Slice;
    case QmitkMxNSyncAxis::Time:        return QmitkMxNAxisGlyph::Time;
    case QmitkMxNSyncAxis::Crosshair:   return QmitkMxNAxisGlyph::Crosshair;
    case QmitkMxNSyncAxis::Selection:   return QmitkMxNAxisGlyph::Selection;
    case QmitkMxNSyncAxis::Windowing:   return QmitkMxNAxisGlyph::Windowing;
    case QmitkMxNSyncAxis::Lut:         return QmitkMxNAxisGlyph::Lut;
  }
  return QmitkMxNAxisGlyph::Pan;
}

/**
 * \brief The axis's display name, untranslated: callers pass it through tr(),
 *        so every surface names an axis alike.
 */
inline const char* QmitkMxNSyncAxisLabel(QmitkMxNSyncAxis axis)
{
  switch (axis)
  {
    case QmitkMxNSyncAxis::Orientation: return "Orientation";
    case QmitkMxNSyncAxis::Pan:         return "Pan";
    case QmitkMxNSyncAxis::Zoom:        return "Zoom";
    case QmitkMxNSyncAxis::Slice:       return "Slice";
    case QmitkMxNSyncAxis::Time:        return "Time";
    case QmitkMxNSyncAxis::Crosshair:   return "Crosshair";
    case QmitkMxNSyncAxis::Selection:   return "Data selection";
    case QmitkMxNSyncAxis::Windowing:   return "Windowing";
    case QmitkMxNSyncAxis::Lut:         return "LUT";
  }
  return "";
}

/** \brief Readable text ink on a fill in 'hue', for every surface that fills
 *         with a group's colour. */
MITKQTWIDGETS_EXPORT QColor QmitkMxNInkFor(const QColor& hue);

/**
 * \brief The opacity, relative to its resting one, at which a synchronization
 *        surface draws its other elements while some of them are highlighted.
 *
 * Fading the rest works for every group colour, where brightening or framing
 * the highlighted elements in white fails on light hues. A group's own linked
 * and unlinked looks keep their order when both fade.
 */
inline constexpr qreal QmitkMxNHighlightFade = 0.35;

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

#endif
