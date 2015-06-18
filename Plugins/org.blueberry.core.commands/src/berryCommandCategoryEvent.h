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


#ifndef BERRYCOMMANDCATEGORYEVENT_H_
#define BERRYCOMMANDCATEGORYEVENT_H_

#include "common/berryAbstractNamedHandleEvent.h"

namespace berry {

class CommandCategory;

/**
 * An instance of this class describes changes to an instance of
 * <code>Category</code>.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @since 3.1
 * @see ICategoryListener#categoryChanged(CategoryEvent)
 */
class BERRY_COMMANDS CommandCategoryEvent : public AbstractNamedHandleEvent {

public:

  berryObjectMacro(CommandCategoryEvent);

  /**
   * Creates a new instance of this class.
   *
   * @param category
   *            the instance of the interface that changed.
   * @param definedChanged
   *            true, iff the defined property changed.
   * @param descriptionChanged
   *            true, iff the description property changed.
   * @param nameChanged
   *            true, iff the name property changed.
   */
  CommandCategoryEvent(const SmartPointer<CommandCategory> category, bool definedChanged,
      bool descriptionChanged, bool nameChanged);

  /**
   * Returns the instance of the interface that changed.
   *
   * @return the instance of the interface that changed. Guaranteed not to be
   *         <code>null</code>.
   */
  SmartPointer<CommandCategory> GetCategory() const;

private:

  /**
   * The category that has changed; this value is never <code>null</code>.
   */
  const SmartPointer<CommandCategory> category;
};

}

#endif /* BERRYCOMMANDCATEGORYEVENT_H_ */
