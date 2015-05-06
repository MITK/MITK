/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYSEPARATOR_H
#define BERRYSEPARATOR_H

#include "internal/berryAbstractGroupMarker.h"

namespace berry {

/**
 * A separator is a special kind of contribution item which acts
 * as a visual separator and, optionally, acts as a group marker.
 * Unlike group markers, separators do have a visual representation for
 * menus and toolbars.
 * <p>
 * This class may be instantiated; it is not intended to be
 * subclassed outside the framework.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class BERRY_UI_QT Separator : public AbstractGroupMarker
{

public:

  /**
   * Creates a separator which does not start a new group.
     */
  Separator();

  /**
   * Creates a new separator which also defines a new group having the given group name.
   * The group name must not be <code>null</code> or the empty string.
   * The group name is also used as the item id.
   *
   * @param groupName the group name of the separator
   */
  Separator(const QString& groupName);

  using AbstractGroupMarker::Fill;

  void Fill(QMenu* menu, QAction* before);

  void Fill(QToolBar* toolbar, QAction* before);

  /**
   * The <code>Separator</code> implementation of this <code>IContributionItem</code>
   * method returns <code>true</code>
   */
  bool IsSeparator() const;
};

}

#endif // BERRYSEPARATOR_H
