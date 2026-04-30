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
  /**
   * \brief Constants for node selection preference keys.
   *
   * Defines preference node paths and value identifiers used by the node
   * selection dialog and its inspectors to persist user preferences such as
   * visible inspectors, preferred inspector, and visibility of special
   * inspectors (favorites, history).
   *
   * \sa QmitkNodeSelectionDialog
   * \sa QmitkNodeSelectionPreferenceHelper
   */
  struct MITKQTWIDGETS_EXPORT NodeSelectionConstants
  {
    /** \brief Preference node path for the root of all node selection preferences. */
    static const std::string ROOT_PREFERENCE_NODE_ID;

    /** \brief Preference sub-node path storing the ordered list of visible inspectors.
     *
     * Each child node is named with the ordering number and stores a
     * VISIBLE_INSPECTOR_ID value identifying the inspector.
     */
    static const std::string VISIBLE_INSPECTORS_NODE_ID;

    /** \brief Key for the preferred (default) inspector ID in the root preference node. */
    static const std::string PREFERRED_INSPECTOR_ID;

    /** \brief Key for the inspector ID stored within each visible inspector preference node. */
    static const std::string VISIBLE_INSPECTOR_ID;

    /** \brief Key controlling whether the favorite inspector is shown. */
    static const std::string SHOW_FAVORITE_INSPECTOR;

    /** \brief Key controlling whether the history inspector is shown. */
    static const std::string SHOW_HISTORY_INSPECTOR;
  };
}

#endif
