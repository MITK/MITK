/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __QMITK_NODE_SELECTION_PREFERENCE_HELPER_H
#define __QMITK_NODE_SELECTION_PREFERENCE_HELPER_H

#include <string>
#include <map>

namespace mitk
{
  using DataStorageInspectorIDType = std::string;

  /** map containing the IDs of all inspectors that should be visible. The map key is the order of appareance
  of the respective inspector.*/
  using VisibleDataStorageInspectorMapType = std::map<unsigned int, DataStorageInspectorIDType>;

    /** Stores the given ID as favorite inspector.*/
    void PutFavoriteDataStorageInspector(const DataStorageInspectorIDType& id);

    /** Gets the ID of the current favorite data storage inspector.
     * If empty string is returned, no favorite is set.*/
    DataStorageInspectorIDType GetFavoriteDataStorageInspector();

    /** Stores the given map of visible inspectors.*/
    void PutVisibleDataStorageInspectors(const VisibleDataStorageInspectorMapType& inspectors);

    /** Gets the map of current visible inspectors.*/
    VisibleDataStorageInspectorMapType GetVisibleDataStorageInspectors();
}

#endif
