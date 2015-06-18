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


#include "berryContributionRoot.h"

#include <berryIContributionItem.h>
#include <berryIEvaluationReference.h>
#include <berryAbstractContributionFactory.h>

#include "berryAlwaysEnabledExpression.h"
#include "berryInternalMenuService.h"

namespace berry {

ContributionRoot::ContributionRoot(InternalMenuService* menuService, const QSet<SmartPointer<IEvaluationReference> > &restriction,
                 ContributionManager* mgr, AbstractContributionFactory* factory)
  : menuService(menuService)
  , mgr(mgr)
  , factory(factory)
  , restriction(restriction)
{
}

void ContributionRoot::AddContributionItem(const SmartPointer<IContributionItem>& item,
                                           const SmartPointer<Expression>& visibleWhen)
{
  if (item.IsNull())
    throw std::invalid_argument("item must not be null");

  topLevelItems.append(item);
  Expression::Pointer visibleWhenTmp = visibleWhen;
  if (visibleWhenTmp.IsNull())
    visibleWhenTmp = AlwaysEnabledExpression::INSTANCE;

  menuService->RegisterVisibleWhen(item, visibleWhenTmp, restriction,
                                   CreateIdentifierId(item));
  itemsToExpressions.append(item);
}

QList<SmartPointer<IContributionItem> > ContributionRoot::GetItems() const
{
  return topLevelItems;
}

void ContributionRoot::Release()
{
  foreach (IContributionItem::Pointer item, itemsToExpressions)
  {
    menuService->UnregisterVisibleWhen(item, restriction);
    //item.dispose();
  }
}

void ContributionRoot::RegisterVisibilityForChild(const SmartPointer<IContributionItem>& item,
                                const SmartPointer<Expression>& visibleWhen)
{
  if (item.IsNull())
    throw std::invalid_argument("item must not be null");

  Expression::Pointer visibleWhenTmp = visibleWhen;
  if (visibleWhenTmp.IsNull())
    visibleWhenTmp = AlwaysEnabledExpression::INSTANCE;

  menuService->RegisterVisibleWhen(item, visibleWhenTmp, restriction,
                                   CreateIdentifierId(item));
  itemsToExpressions.append(item);
}

ContributionManager* ContributionRoot::GetManager() const
{
  return mgr;
}

QString ContributionRoot::CreateIdentifierId(const SmartPointer<IContributionItem>& item)
{
  QString namespaze = factory->GetNamespace();
  // create the activity identifier ID. If this factory doesn't have a namespace
  // it will be null.
  QString identifierID = (!namespaze.isEmpty()) ? namespaze + '/' + item->GetId()
                                                : QString();
  return identifierID;
}

}
