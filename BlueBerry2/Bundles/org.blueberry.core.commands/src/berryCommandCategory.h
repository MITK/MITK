/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYCOMMANDCATEGORY_H_
#define BERRYCOMMANDCATEGORY_H_

#include "common/berryNamedHandleObject.h"
#include "berryICommandCategoryListener.h"

namespace berry
{

using namespace osgi::framework;

/**
 * <p>
 * A logical group for a set of commands. A command belongs to exactly one
 * category. The category has no functional effect, but may be used in graphical
 * tools that want to group the set of commands somehow.
 * </p>
 *
 * @since 3.1
 */
class CommandCategory: public NamedHandleObject
{

public:

  osgiObjectMacro(CommandCategory)

  /**
   * Adds a listener to this category that will be notified when this
   * category's state changes.
   *
   * @param categoryListener
   *            The listener to be added; must not be <code>null</code>.
   */
void  AddCategoryListener(
      ICommandCategoryListener::Pointer categoryListener);

  /**
   * <p>
   * Defines this category by giving it a name, and possibly a description as
   * well. The defined property automatically becomes <code>true</code>.
   * </p>
   * <p>
   * Notification is sent to all listeners that something has changed.
   * </p>
   *
   * @param name
   *            The name of this command; must not be <code>null</code>.
   * @param description
   *            The description for this command; may be <code>null</code>.
   */
  void Define(const std::string& name, const std::string& description);

  /**
   * Removes a listener from this category.
   *
   * @param categoryListener
   *            The listener to be removed; must not be <code>null</code>.
   *
   */
  void RemoveCategoryListener(
      ICommandCategoryListener::Pointer categoryListener);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.core.commands.common.HandleObject#toString()
   */
  std::string ToString();

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.core.commands.common.HandleObject#undefine()
   */
  void Undefine();

protected:

  friend class CommandManager;

  /**
   * Constructs a new instance of <code>Category</code> based on the given
   * identifier. When a category is first constructed, it is undefined.
   * Category should only be constructed by the <code>CommandManager</code>
   * to ensure that identifier remain unique.
   *
   * @param id
   *            The identifier for the category. This value must not be
   *            <code>null</code>, and must be unique amongst all
   *            categories.
   */
  CommandCategory(const std::string& id);

private:

  /**
   * Notifies the listeners for this category that it has changed in some way.
   *
   * @param categoryEvent
   *            The event to send to all of the listener; must not be
   *            <code>null</code>.
   */
  void FireCategoryChanged(const SmartPointer<CommandCategoryEvent> categoryEvent);

  /**
   * A collection of objects listening to changes to this category. This
   * collection is <code>null</code> if there are no listeners.
   */
  ICommandCategoryListener::Events categoryEvents;
};

}

#endif /* BERRYCOMMANDCATEGORY_H_ */
