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
  using DataStorageInspectorIDType = std::string;

  /** map containing the IDs of all inspectors that should be visible. The map key is the order of appearance
  of the respective inspector.*/
  using VisibleDataStorageInspectorMapType = std::map<unsigned int, DataStorageInspectorIDType>;

  /** Stores the given ID as favorite inspector.*/
  MITKQTWIDGETS_EXPORT void PutPreferredDataStorageInspector(const DataStorageInspectorIDType& id);

  /** Gets the ID of the current favorite data storage inspector.
   * If empty string is returned, no favorite is set.*/
  MITKQTWIDGETS_EXPORT DataStorageInspectorIDType GetPreferredDataStorageInspector();

  /** Stores the given map of visible inspectors.*/
  MITKQTWIDGETS_EXPORT void PutVisibleDataStorageInspectors(const VisibleDataStorageInspectorMapType& inspectors);

  /** Gets the map of current visible inspectors.*/
  MITKQTWIDGETS_EXPORT VisibleDataStorageInspectorMapType GetVisibleDataStorageInspectors();

  /** Stores the given show state of the favorite inspector.*/
  MITKQTWIDGETS_EXPORT void PutShowFavoritesInspector(bool show);

  /** Indicates if the favorites inspector should be shown. */
  MITKQTWIDGETS_EXPORT bool GetShowFavoritesInspector();

  /** Stores the given show state of the history inspector.*/
  MITKQTWIDGETS_EXPORT void PutShowHistoryInspector(bool show);

  /** Indicates if the history inspector should be shown. */
  MITKQTWIDGETS_EXPORT bool GetShowHistoryInspector();
}

#endif
