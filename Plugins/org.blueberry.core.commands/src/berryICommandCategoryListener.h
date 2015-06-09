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


#ifndef BERRYICOMMANDCATEGORYLISTENER_H_
#define BERRYICOMMANDCATEGORYLISTENER_H_

#include <berryMessage.h>

namespace berry {

template<class T> class SmartPointer;

class CommandCategoryEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>Category</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see CommandCategory#AddCategoryListener(ICommandCategoryListener)
 * @see CommandCategory#RemoveCategoryListener(ICommandCategoryListener)
 */
struct ICommandCategoryListener
{

  struct Events {

    typedef Message1<const SmartPointer<const CommandCategoryEvent>& > Event;

    Event categoryChanged;

    void AddListener(ICommandCategoryListener* listener);
    void RemoveListener(ICommandCategoryListener* listener);


    typedef MessageDelegate1<ICommandCategoryListener, const SmartPointer<const CommandCategoryEvent>& > Delegate;
  };

  virtual ~ICommandCategoryListener();

  /**
   * Notifies that one or more properties of an instance of
   * <code>CommandCategory</code> have changed. Specific details are described in
   * the <code>CommandCategoryEvent</code>.
   *
   * @param categoryEvent
   *            the category event. Guaranteed not to be <code>null</code>.
   */
  virtual void CategoryChanged(const SmartPointer<const CommandCategoryEvent>& categoryEvent) = 0;
};

}


#endif /* BERRYICOMMANDCATEGORYLISTENER_H_ */
