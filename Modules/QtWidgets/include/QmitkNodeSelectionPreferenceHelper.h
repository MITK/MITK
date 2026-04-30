/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNodeSelectionPreferenceHelper_h
#define QmitkNodeSelectionPreferenceHelper_h

#include <MitkQtWidgetsExports.h>

#include <string>
#include <map>

namespace mitk
{
  /** \brief Type alias for data storage inspector identifiers. */
  using DataStorageInspectorIDType = std::string;

  /**
   * \brief Map of visible data storage inspectors, keyed by display order.
   *
   * The map key is the ordering number (0-based) that determines the tab order
   * in the node selection dialog. The value is the inspector's unique ID.
   */
  using VisibleDataStorageInspectorMapType = std::map<unsigned int, DataStorageInspectorIDType>;

  /**
   * \brief Stores the given inspector ID as the preferred (default) inspector.
   * \param[in] id The inspector ID to set as preferred.
   */
  MITKQTWIDGETS_EXPORT void PutPreferredDataStorageInspector(const DataStorageInspectorIDType& id);

  /**
   * \brief Returns the ID of the preferred data storage inspector.
   * \return The preferred inspector ID, or an empty string if none is set.
   */
  MITKQTWIDGETS_EXPORT DataStorageInspectorIDType GetPreferredDataStorageInspector();

  /**
   * \brief Persists the given map of visible inspectors to preferences.
   * \param[in] inspectors The map of ordered inspector IDs.
   */
  MITKQTWIDGETS_EXPORT void PutVisibleDataStorageInspectors(const VisibleDataStorageInspectorMapType& inspectors);

  /**
   * \brief Retrieves the currently configured visible inspectors from preferences.
   * \return The map of ordered inspector IDs.
   */
  MITKQTWIDGETS_EXPORT VisibleDataStorageInspectorMapType GetVisibleDataStorageInspectors();

  /**
   * \brief Persists whether the favorites inspector should be shown.
   * \param[in] show True to show the favorites inspector.
   */
  MITKQTWIDGETS_EXPORT void PutShowFavoritesInspector(bool show);

  /**
   * \brief Returns whether the favorites inspector should be shown.
   * \return True if the favorites inspector is configured to be visible.
   */
  MITKQTWIDGETS_EXPORT bool GetShowFavoritesInspector();

  /**
   * \brief Persists whether the history inspector should be shown.
   * \param[in] show True to show the history inspector.
   */
  MITKQTWIDGETS_EXPORT void PutShowHistoryInspector(bool show);

  /**
   * \brief Returns whether the history inspector should be shown.
   * \return True if the history inspector is configured to be visible.
   */
  MITKQTWIDGETS_EXPORT bool GetShowHistoryInspector();
}

#endif
