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


#ifndef BERRYICOMMANDCATEGORYLISTENER_H_
#define BERRYICOMMANDCATEGORYLISTENER_H_

#include <berryObject.h>
#include <berryMacros.h>
#include <berryMessage.h>

namespace berry {

class CommandCategoryEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>Category</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @since 3.1
 * @see CommandCategory#AddCategoryListener(ICommandCategoryListener)
 * @see CommandCategory#RemoveCategoryListener(ICommandCategoryListener)
 */
struct ICommandCategoryListener : public virtual Object {

  berryInterfaceMacro(ICommandCategoryListener, berry)

  struct Events {

    typedef Message1<const SmartPointer<const CommandCategoryEvent> > Event;

    Event categoryChanged;

    void AddListener(ICommandCategoryListener::Pointer listener);
    void RemoveListener(ICommandCategoryListener::Pointer listener);


    typedef MessageDelegate1<ICommandCategoryListener, const SmartPointer<const CommandCategoryEvent> > Delegate;
  };

  /**
   * Notifies that one or more properties of an instance of
   * <code>CommandCategory</code> have changed. Specific details are described in
   * the <code>CommandCategoryEvent</code>.
   *
   * @param categoryEvent
   *            the category event. Guaranteed not to be <code>null</code>.
   */
  virtual void CategoryChanged(const SmartPointer<const CommandCategoryEvent> categoryEvent) = 0;
};

}


#endif /* BERRYICOMMANDCATEGORYLISTENER_H_ */
