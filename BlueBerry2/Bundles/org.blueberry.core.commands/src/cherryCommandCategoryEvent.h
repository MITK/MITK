/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYCOMMANDCATEGORYEVENT_H_
#define CHERRYCOMMANDCATEGORYEVENT_H_

#include "common/cherryAbstractNamedHandleEvent.h"

namespace cherry {

using namespace osgi::framework;

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
class CHERRY_COMMANDS CommandCategoryEvent : public AbstractNamedHandleEvent {

public:

  osgiObjectMacro(CommandCategoryEvent)

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

#endif /* CHERRYCOMMANDCATEGORYEVENT_H_ */
