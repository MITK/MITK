/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNodeSelectionConstants_h
#define QmitkNodeSelectionConstants_h

#include <MitkQtWidgetsExports.h>

#include <string>

namespace mitk
{
  struct MITKQTWIDGETS_EXPORT NodeSelectionConstants
  {
    /** ID/Path of main preference node for node selections.*/
    static const std::string ROOT_PREFERENCE_NODE_ID;
    /** ID of main preference node where all visible inspectors are stored (e.g. ROOT_PREFERENCE_NODE_ID+"/"+VISIBLE_INSPECTORS_NODE_ID+"/[orderering #]").
    The sub node naming encodes the ordering number of the visible inspector.*/
    static const std::string VISIBLE_INSPECTORS_NODE_ID;
    /** ID for the value that stores the preferred inspector ID in the root preference node.*/
    static const std::string PREFERRED_INSPECTOR_ID;
    /** ID for the value that stores the inspector ID in the preference node.*/
    static const std::string VISIBLE_INSPECTOR_ID;
    /** ID for the value that stores if the favorite inspector should be visible.*/
    static const std::string SHOW_FAVORITE_INSPECTOR;
    /** ID for the value that stores if the history inspector should be visible.*/
    static const std::string SHOW_HISTORY_INSPECTOR;
  };
}

#endif
