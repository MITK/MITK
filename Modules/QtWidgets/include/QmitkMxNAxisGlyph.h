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

class QColor;
class QPixmap;

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
 *        pixmap.
 *
 * The glyphs are embedded SVG resources whose placeholder color (`#00ff00`,
 * the same convention QmitkStyleManager uses) is swapped for 'color' at load,
 * so a glyph can take any foreground (a group hue, a grayed decoupled state)
 * and stay crisp at any size. Returns a null pixmap if the resource is missing.
 */
MITKQTWIDGETS_EXPORT QPixmap QmitkMxNRenderAxisGlyph(QmitkMxNAxisGlyph glyph, const QColor& color, int sizePx);

#endif
