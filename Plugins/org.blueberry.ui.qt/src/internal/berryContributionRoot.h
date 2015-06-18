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


#ifndef BERRYCONTRIBUTIONROOT_H
#define BERRYCONTRIBUTIONROOT_H

#include <berryIContributionRoot.h>

#include <QSet>

namespace berry {

struct IEvaluationReference;
struct InternalMenuService;

class ContributionManager;
class AbstractContributionFactory;

/**
 * Default implementation.
 */
class ContributionRoot : public IContributionRoot
{

private:

  QList<SmartPointer<IContributionItem> > topLevelItems;
  QList<SmartPointer<IContributionItem> > itemsToExpressions;
  InternalMenuService* menuService;
  ContributionManager* mgr;
  AbstractContributionFactory* factory;

public:

  QSet<SmartPointer<IEvaluationReference> > restriction;

  berryObjectMacro(berry::ContributionRoot)

  ContributionRoot(InternalMenuService* menuService, const QSet<SmartPointer<IEvaluationReference> >& restriction,
                   ContributionManager* mgr, AbstractContributionFactory* factory);

  /*
   * @see IContributionRoot#AddContributionItem(IContributionItem, Expression)
   */
  void AddContributionItem(const SmartPointer<IContributionItem>& item,
                           const SmartPointer<Expression>& visibleWhen) override;

  QList<SmartPointer<IContributionItem> > GetItems() const;

  /**
   * Unregister all visible when expressions from the menu service.
   */
  void Release();

  /*
   * @see IContributionRoot#RegisterVisibilityForChild(IContributionItem, Expression)
   */
  void RegisterVisibilityForChild(const SmartPointer<IContributionItem>& item,
                                  const SmartPointer<Expression>& visibleWhen) override;

  /**
   * @return Returns the mgr.
   */
  ContributionManager* GetManager() const;

private:

  /**
     * Create the activity identifier for this contribution item.
     *
   * @param item the item
   * @return the identifier
   */
  QString CreateIdentifierId(const SmartPointer<IContributionItem>& item);

};

}

#endif // BERRYCONTRIBUTIONROOT_H
