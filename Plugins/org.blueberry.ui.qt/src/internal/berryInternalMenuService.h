/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYINTERNALMENUSERVICE_H
#define BERRYINTERNALMENUSERVICE_H

#include <berryIMenuService.h>

#include <QUrl>

namespace berry {

struct IContributionItem;
struct IEvaluationReference;
struct IServiceLocator;

class Expression;
class MenuAdditionCacheEntry;

/**
 * Internal baseclass for Workbench and Window menu service implementations.
 * Methods in this class might some day make sense to live in IMenuService.
 */
struct InternalMenuService : public IMenuService
{

  /**
   * Ties an expression to {@link IContributionItem#SetVisible(bool)}.
   *
   * @param item
   *            the item
   * @param visibleWhen
   *            the expression
   * @param restriction
   *            the restriction expression
   * @param identifierId
   *         the activity identifier id
   */
  virtual void RegisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                                   const SmartPointer<Expression>& visibleWhen,
                                   QSet<SmartPointer<IEvaluationReference> >& restriction,
                                   const QString& identifierID) = 0;

  /**
   * Removes any expressions bound to
   * {@link IContributionItem#SetVisible(bool)} of the given item
   *
   * @param item
   *            the item to unbind
   */
  virtual void UnregisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                                     QSet<SmartPointer<IEvaluationReference> >& restriction) = 0;

  /**
   * Return a list of {@link MenuAdditionCacheEntry} objects that are
   * contributed at the given uri.
   *
   * @param uri
   *            the uri to search on
   * @return the list of items
   */
  virtual QList<SmartPointer<AbstractContributionFactory> > GetAdditionsForURI(const QUrl& uri) = 0;

  virtual void PopulateContributionManager(IServiceLocator* serviceLocatorToUse,
                                           const QSet<SmartPointer<IEvaluationReference> >& restriction,
                                           ContributionManager* mgr,
                                           const QString& uri, bool recurse) = 0;

  virtual void PopulateContributionManager(ContributionManager* mgr,
                                           const QString& uri, bool recurse) = 0;

  using IMenuService::PopulateContributionManager;
};

}

#endif // BERRYINTERNALMENUSERVICE_H
