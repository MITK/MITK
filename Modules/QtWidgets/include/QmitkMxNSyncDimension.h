/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNSyncDimension_h
#define QmitkMxNSyncDimension_h

#include <array>
#include <optional>
#include <string>

/**
 * \brief Synchronization dimensions a cell of the MxN editor can link
 *        per-group via the layout document's `links.<dim>` keys.
 *
 * The `selection` dimension is not part of this enum: it predates the
 * per-dimension model, has its own engine (QmitkSynchronizedWidgetConnector)
 * and its own per-cell control, and is handled by dedicated code paths.
 */
enum class QmitkMxNSyncDimension
{
  Pan,
  Zoom,
  Slice,
  Crosshair,
  Orientation,
  Windowing,
  Lut
};

inline constexpr std::array<QmitkMxNSyncDimension, 7> QmitkMxNAllSyncDimensions{
  QmitkMxNSyncDimension::Pan,       QmitkMxNSyncDimension::Zoom,      QmitkMxNSyncDimension::Slice,
  QmitkMxNSyncDimension::Crosshair, QmitkMxNSyncDimension::Orientation,
  QmitkMxNSyncDimension::Windowing, QmitkMxNSyncDimension::Lut
};

/** \brief The dimension's `links` key in the v3 layout document. */
inline const char* QmitkMxNSyncDimensionToLinkKey(QmitkMxNSyncDimension dimension)
{
  switch (dimension)
  {
    case QmitkMxNSyncDimension::Pan:         return "pan";
    case QmitkMxNSyncDimension::Zoom:        return "zoom";
    case QmitkMxNSyncDimension::Slice:       return "slice";
    case QmitkMxNSyncDimension::Crosshair:   return "crosshair";
    case QmitkMxNSyncDimension::Orientation: return "orientation";
    case QmitkMxNSyncDimension::Windowing:   return "windowing";
    case QmitkMxNSyncDimension::Lut:         return "lut";
  }
  return "";
}

/** \brief Parse a v3 `links` key; empty result for unknown keys (including "selection"). */
inline std::optional<QmitkMxNSyncDimension> QmitkMxNSyncDimensionFromLinkKey(const std::string& key)
{
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    if (key == QmitkMxNSyncDimensionToLinkKey(dimension))
    {
      return dimension;
    }
  }
  return std::nullopt;
}

#endif
