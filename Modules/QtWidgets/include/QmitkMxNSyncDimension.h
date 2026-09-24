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
#include <cstddef>
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

/**
 * \brief The axes the MxN synchronization surfaces (barcodes, advanced
 *        matrix, sync peek plates) present: the seven synchronization
 *        dimensions followed by data selection, in slot order.
 */
enum class QmitkMxNSyncAxis
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

inline constexpr int QmitkMxNSyncAxisCount = static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 1;

static_assert(static_cast<int>(QmitkMxNSyncAxis::Selection) + 1 == QmitkMxNSyncAxisCount,
              "QmitkMxNSyncAxis must list the synchronization dimensions, then Selection");
static_assert([] {
                for (std::size_t i = 0; i < QmitkMxNAllSyncDimensions.size(); ++i)
                {
                  if (static_cast<std::size_t>(QmitkMxNAllSyncDimensions[i]) != i)
                  {
                    return false;
                  }
                }
                return true;
              }(),
              "QmitkMxNAllSyncDimensions must follow the QmitkMxNSyncDimension declaration order");

/** \brief The axis's position on the synchronization surfaces. */
inline constexpr int QmitkMxNSyncAxisToSlot(QmitkMxNSyncAxis axis)
{
  return static_cast<int>(axis);
}

/** \brief The axis at a surface position; empty outside [0, QmitkMxNSyncAxisCount). */
inline constexpr std::optional<QmitkMxNSyncAxis> QmitkMxNSyncAxisFromSlot(int slot)
{
  if (slot < 0 || slot >= QmitkMxNSyncAxisCount)
  {
    return std::nullopt;
  }
  return static_cast<QmitkMxNSyncAxis>(slot);
}

/** \brief The dimension an axis synchronizes; empty for Selection. */
inline constexpr std::optional<QmitkMxNSyncDimension> QmitkMxNSyncAxisDimension(QmitkMxNSyncAxis axis)
{
  if (QmitkMxNSyncAxis::Selection == axis)
  {
    return std::nullopt;
  }
  return static_cast<QmitkMxNSyncDimension>(static_cast<int>(axis));
}

/** \brief The axis presenting a dimension. */
inline constexpr QmitkMxNSyncAxis QmitkMxNSyncAxisOf(QmitkMxNSyncDimension dimension)
{
  return static_cast<QmitkMxNSyncAxis>(static_cast<int>(dimension));
}

/**
 * \brief How a sync peek plate lays out its eight axis glyphs: one row of
 *        eight, or two rows of four in reading order (pan, zoom, slice,
 *        crosshair above orientation, windowing, LUT, selection). One
 *        arrangement serves every plate of a layout, so an axis sits at the
 *        same place in every cell.
 */
enum class QmitkMxNPeekRows
{
  One,
  Two
};

/** \brief The glyphs per row of 'rows'. */
inline constexpr int QmitkMxNPeekColumns(QmitkMxNPeekRows rows)
{
  return QmitkMxNPeekRows::One == rows ? QmitkMxNSyncAxisCount : QmitkMxNSyncAxisCount / 2;
}

#endif
