/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNSyncDimension_h
#define QmitkMxNSyncDimension_h

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

/**
 * \brief Synchronization dimensions a cell of the MxN editor can link
 *        per-group via the layout document's `links.<dim>` keys.
 *
 * The `selection` dimension is not part of this enum: it predates the
 * per-dimension model, has its own engine (QmitkSynchronizedWidgetConnector),
 * and is handled by dedicated code paths.
 *
 * Geometry is not a dimension either. A slice step or a plane name only means
 * the same physical location within one reference geometry, so which cells
 * share a geometry follows from their slice and orientation links (see
 * QmitkMxNMultiWidget::ReinitSyncGroupGeometry); linking it independently
 * would allow slice-linked cells whose steps mean different places.
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
 * \brief The id of the reserved group whose members follow the application's
 *        value (its time, for example) instead of a value of their own.
 *
 * The group always exists, takes no entry in a layout's `groups` dict, and is
 * reserved in every spelling: no user group may be named "Global" or "GLOBAL".
 */
inline constexpr const char* QmitkMxNGlobalGroupId = "global";

/**
 * \brief The `links` keys whose links may target the reserved group 'global'.
 *
 * The one table the loader, the engine API and the layout editor consult, so
 * that enabling 'global' on another axis means adding its key here and giving
 * the axis the semantics of following the application's value. Keyed by link
 * key, the vocabulary the loader works in; the engine maps a dimension with
 * QmitkMxNSyncDimensionToLinkKey, the editor an axis with
 * QmitkMxNSyncAxisToLinkKey.
 */
inline constexpr std::array<std::string_view, 1> QmitkMxNLinkKeysAcceptingGlobal{ "time" };

/** \brief Whether links under 'linkKey' may target the reserved group 'global'. */
inline bool QmitkMxNLinkKeyAcceptsGlobal(std::string_view linkKey)
{
  return std::find(QmitkMxNLinkKeysAcceptingGlobal.begin(), QmitkMxNLinkKeysAcceptingGlobal.end(), linkKey)
         != QmitkMxNLinkKeysAcceptingGlobal.end();
}

/**
 * \brief The axes the MxN synchronization surfaces present (barcodes, group-card
 *        strips, the advanced matrix, sync peek plates), in slot order: three
 *        families of three - View (how the window looks at its data:
 *        orientation, pan, zoom), Position (where in the data: slice, time,
 *        crosshair) and Content (what is shown and how: selection, windowing,
 *        LUT).
 *
 * The order is presentation only; the layout document addresses links by key.
 * Selection and Time have no QmitkMxNSyncDimension: selection has its own
 * engine, and every window follows the application's time through the
 * reserved group 'global' until time groups other than 'global' exist, so
 * there is nothing to store.
 */
enum class QmitkMxNSyncAxis
{
  Orientation,
  Pan,
  Zoom,
  Slice,
  Time,
  Crosshair,
  Selection,
  Windowing,
  Lut
};

inline constexpr std::array<QmitkMxNSyncAxis, 9> QmitkMxNAllSyncAxes{
  QmitkMxNSyncAxis::Orientation, QmitkMxNSyncAxis::Pan,       QmitkMxNSyncAxis::Zoom,
  QmitkMxNSyncAxis::Slice,       QmitkMxNSyncAxis::Time,      QmitkMxNSyncAxis::Crosshair,
  QmitkMxNSyncAxis::Selection,   QmitkMxNSyncAxis::Windowing, QmitkMxNSyncAxis::Lut
};

inline constexpr int QmitkMxNSyncAxisCount = static_cast<int>(QmitkMxNAllSyncAxes.size());

static_assert([] {
                for (std::size_t i = 0; i < QmitkMxNAllSyncAxes.size(); ++i)
                {
                  if (static_cast<std::size_t>(QmitkMxNAllSyncAxes[i]) != i)
                  {
                    return false;
                  }
                }
                return true;
              }(),
              "QmitkMxNAllSyncAxes must follow the QmitkMxNSyncAxis declaration order: a slot is its enum value");

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
  return QmitkMxNAllSyncAxes[static_cast<std::size_t>(slot)];
}

/** \brief The dimension an axis synchronizes; empty for Selection and Time. */
inline constexpr std::optional<QmitkMxNSyncDimension> QmitkMxNSyncAxisDimension(QmitkMxNSyncAxis axis)
{
  switch (axis)
  {
    case QmitkMxNSyncAxis::Orientation: return QmitkMxNSyncDimension::Orientation;
    case QmitkMxNSyncAxis::Pan:         return QmitkMxNSyncDimension::Pan;
    case QmitkMxNSyncAxis::Zoom:        return QmitkMxNSyncDimension::Zoom;
    case QmitkMxNSyncAxis::Slice:       return QmitkMxNSyncDimension::Slice;
    case QmitkMxNSyncAxis::Crosshair:   return QmitkMxNSyncDimension::Crosshair;
    case QmitkMxNSyncAxis::Windowing:   return QmitkMxNSyncDimension::Windowing;
    case QmitkMxNSyncAxis::Lut:         return QmitkMxNSyncDimension::Lut;
    case QmitkMxNSyncAxis::Time:
    case QmitkMxNSyncAxis::Selection:   return std::nullopt;
  }
  return std::nullopt;
}

/** \brief The axis presenting a dimension. */
inline constexpr QmitkMxNSyncAxis QmitkMxNSyncAxisOf(QmitkMxNSyncDimension dimension)
{
  switch (dimension)
  {
    case QmitkMxNSyncDimension::Pan:         return QmitkMxNSyncAxis::Pan;
    case QmitkMxNSyncDimension::Zoom:        return QmitkMxNSyncAxis::Zoom;
    case QmitkMxNSyncDimension::Slice:       return QmitkMxNSyncAxis::Slice;
    case QmitkMxNSyncDimension::Crosshair:   return QmitkMxNSyncAxis::Crosshair;
    case QmitkMxNSyncDimension::Orientation: return QmitkMxNSyncAxis::Orientation;
    case QmitkMxNSyncDimension::Windowing:   return QmitkMxNSyncAxis::Windowing;
    case QmitkMxNSyncDimension::Lut:         return QmitkMxNSyncAxis::Lut;
  }
  return QmitkMxNSyncAxis::Pan;
}

/** \brief The axis's `links` key in the v3 layout document ("time", "selection", ...). */
inline const char* QmitkMxNSyncAxisToLinkKey(QmitkMxNSyncAxis axis)
{
  switch (axis)
  {
    case QmitkMxNSyncAxis::Time:      return "time";
    case QmitkMxNSyncAxis::Selection: return "selection";
    default:                          return QmitkMxNSyncDimensionToLinkKey(*QmitkMxNSyncAxisDimension(axis));
  }
}

#endif
