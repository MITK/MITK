/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYICONTRIBUTIONROOT_H
#define BERRYICONTRIBUTIONROOT_H

#include <berryObject.h>

namespace berry {

struct IContributionItem;

class Expression;

/**
 * Instances of this interface represent a position in the contribution
 * hierarchy into which {@link AbstractContributionFactory} instances may insert
 * elements. Instances of this interface are provided by the platform and this
 * interface should <b>NOT</b> be implemented by clients.
 */
struct IContributionRoot : public Object
{
  berryObjectMacro(berry::IContributionRoot);

  /**
   * Adds a given contribution item with provided visibility expression and
   * kill-switch filtering as a direct child of this container. This should be
   * called for all top-level elements created in
   * berry::AbstractContributionFactory::CreateContributionItems().
   *
   * @param item
   *            the item to add
   * @param visibleWhen
   *            the visibility expression. May be <code>null</code>.
   */
  virtual void AddContributionItem(const SmartPointer<IContributionItem>& item,
                                   const SmartPointer<Expression>& visibleWhen) = 0;

  /**
   * Registers visibilty for arbitrary {@link IContributionItem} instances
   * that are <b>NOT</b> direct children of this container. Ie: children of a
   * {@link IContributionManager} that has been previously registered with a
   * call to AddContributionItem().
   *
   * @param item
   *            the item for which to register a visibility clause
   * @param visibleWhen
   *            the visibility expression. May be <code>null</code> in which
   *            case this method is a no-op.
   */
  virtual void RegisterVisibilityForChild(const SmartPointer<IContributionItem>& item,
                                          const SmartPointer<Expression>& visibleWhen) = 0;
};

}

#endif // BERRYICONTRIBUTIONROOT_H
