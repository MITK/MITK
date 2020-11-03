/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySlaveMenuService.h"

#include "berryIEvaluationContext.h"
#include "berryIEvaluationReference.h"
#include "berryISourceProvider.h"

#include "berryAbstractContributionFactory.h"

namespace berry {

SlaveMenuService::SlaveMenuService(InternalMenuService* parent,
                 IServiceLocator* serviceLocator,
                 const QSet<SmartPointer<IEvaluationReference> >& restrictions)
  : parent(parent)
  , serviceLocator(serviceLocator)
  , restrictionExpression(restrictions)
{
}

void SlaveMenuService::PopulateContributionManager(ContributionManager* mgr, const QString& uri)
{
  parent->PopulateContributionManager(serviceLocator,
                                      restrictionExpression, mgr, uri, true);
}

void SlaveMenuService::PopulateContributionManager(ContributionManager* mgr,
                                                   const QString& uri, bool recurse)
{
  parent->PopulateContributionManager(serviceLocator,
                                      restrictionExpression, mgr, uri, recurse);
}

SmartPointer<IEvaluationContext> SlaveMenuService::GetCurrentState() const
{
  return parent->GetCurrentState();
}

void SlaveMenuService::AddContributionFactory(const SmartPointer<AbstractContributionFactory>& cache)
{
  if (!factories.contains(cache)) {
    factories.push_back(cache);
  }
  parent->AddContributionFactory(cache);
}

void SlaveMenuService::ReleaseContributions(ContributionManager* mgr)
{
  parent->ReleaseContributions(mgr);
}

void SlaveMenuService::RemoveContributionFactory(const SmartPointer<AbstractContributionFactory>& factory)
{
  factories.removeAll(factory);
  parent->RemoveContributionFactory(factory);
}

void SlaveMenuService::Dispose()
{
  if (!providers.isEmpty())
  {
    for (int i = 0; i < providers.size(); i++)
    {
      parent->RemoveSourceProvider(providers[i]);
    }
    providers.clear();
  }
  if (!factories.isEmpty())
  {
    for (int i = 0; i < factories.size(); i++)
    {
      parent->RemoveContributionFactory(factories[i]);
    }
    factories.clear();
  }
  restrictionExpression.clear();
}

void SlaveMenuService::AddSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  if (!providers.contains(provider))
  {
    providers.push_back(provider);
  }
  parent->AddSourceProvider(provider);
}

void SlaveMenuService::RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  providers.removeAll(provider);
  parent->RemoveSourceProvider(provider);
}

QList<SmartPointer<AbstractContributionFactory> > SlaveMenuService::GetAdditionsForURI(const QUrl& uri)
{
  return parent->GetAdditionsForURI(uri);
}

void SlaveMenuService::RegisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                                           const SmartPointer<Expression>& visibleWhen,
                                           QSet<SmartPointer<IEvaluationReference> >& restriction,
                                           const QString& identifierID)
{
  parent->RegisterVisibleWhen(item, visibleWhen, restriction, identifierID);
}

void SlaveMenuService::UnregisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                                             QSet<SmartPointer<IEvaluationReference> >& restriction)
{
  parent->UnregisterVisibleWhen(item, restriction);
}

void SlaveMenuService::PopulateContributionManager(
    IServiceLocator* serviceLocatorToUse, const QSet<SmartPointer<IEvaluationReference> >& restriction,
    ContributionManager* mgr, const QString& uri, bool recurse)
{
  parent->PopulateContributionManager(serviceLocatorToUse, restriction,
                                      mgr, uri, recurse);
}

}
