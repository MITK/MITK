/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef __QMITK_NODE_SELECTION_PREFERENCE_HELPER_H
#define __QMITK_NODE_SELECTION_PREFERENCE_HELPER_H

#include <string>
#include <map>

namespace mitk
{
  using DataStorageInspectorIDType = std::string;

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
