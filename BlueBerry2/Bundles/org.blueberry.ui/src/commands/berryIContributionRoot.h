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


#ifndef BERRYICONTRIBUTIONROOT_H_
#define BERRYICONTRIBUTIONROOT_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../berryUiDll.h"

namespace berry {

struct IContributionItem;
class Expression;

/**
 * Instances of this interface represent a position in the contribution
 * hierarchy into which {@link AbstractContributionFactory} instances may insert
 * elements. Instances of this interface are provided by the platform and this
 * interface should <b>NOT</b> be implemented by clients.
 *
 *
 * @since 3.3
 */
struct BERRY_UI IContributionRoot : public Object {

  osgiInterfaceMacro(berry::IContributionRoot)

  /**
   * Adds a given contribution item with provided visibility expression and
   * kill-switch filtering as a direct child of this container. This should be
   * called for all top-level elements created in
   * {@link AbstractContributionFactory#createContributionItems(org.eclipse.ui.services.IServiceLocator, IContributionRoot)}
   *
   * @param item
   *            the item to add
   * @param visibleWhen
   *            the visibility expression. May be <code>null</code>.
   */
  virtual void AddContributionItem(SmartPointer<IContributionItem> item,
      SmartPointer<Expression> visibleWhen) = 0;

  /**
   * Registers visibilty for arbitrary {@link IContributionItem} instances
   * that are <b>NOT</b> direct children of this container. Ie: children of a
   * {@link IContributionManager} that has been previously registered with a
   * call to {{@link #addContributionItem(IContributionItem, Expression)}.
   *
   * @param item
   *            the item for which to register a visibility clause
   * @param visibleWhen
   *            the visibility expression. May be <code>null</code> in which
   *            case this method is a no-op.
   */
  virtual void RegisterVisibilityForChild(SmartPointer<IContributionItem> item,
      SmartPointer<Expression> visibleWhen) = 0;
};

}

#endif /* BERRYICONTRIBUTIONROOT_H_ */
