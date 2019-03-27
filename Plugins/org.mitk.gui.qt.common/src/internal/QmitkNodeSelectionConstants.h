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

#ifndef _QMITK_NODE_SELECTION_CONSTANTS_H_
#define _QMITK_NODE_SELECTION_CONSTANTS_H_

#include <string>

#include "org_mitk_gui_qt_common_Export.h"

namespace mitk
{
struct MITK_QT_COMMON NodeSelectionConstants
{
  /** ID/Path of main preference node for node selections.  */
  static const std::string ROOT_PREFERENCE_NODE_ID;

  /** ID of main preference node where all visible inspectors are stored (e.g. ROOT_PREFERENCE_NODE_ID+"/"+VISIBLE_INSPECTORS_NODE_ID+"/[orderering #]").
   The sub node naming encodes the ordering number of the visible inspector.*/
  static const std::string VISIBLE_INSPECTORS_NODE_ID;
  /** ID for the value that stores the favorite inspector ID in the root preference node.*/
  static const std::string FAVORITE_INSPECTOR_ID;
  /** ID for the value that stores the inspector ID in the preference node.*/
  static const std::string VISIBLE_INSPECTOR_ID;
};

}

#endif
