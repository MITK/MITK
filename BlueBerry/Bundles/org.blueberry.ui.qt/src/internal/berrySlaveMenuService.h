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

#ifndef BERRYSLAVEMENUSERVICE_H
#define BERRYSLAVEMENUSERVICE_H

#include "berryInternalMenuService.h"

namespace berry {

/**
 * <p>
 * Provides services related to contributing menu elements to a workbench
 * window. Visibility and showing are tracked at the workbench window level.
 * </p>
 * <p>
 * This class is only intended for internal use within the
 * <code>org.eclipse.ui.workbench</code> plug-in.
 * </p>
 */
class SlaveMenuService : public InternalMenuService
{

private:

  QList<SmartPointer<ISourceProvider> > providers;
  QList<SmartPointer<AbstractContributionFactory> > factories;

  /**
   * The parent menu service for this window. This parent must track menu
   * definitions and the registry. Must not be <code>null</code>
   */
  InternalMenuService* const parent;
  IServiceLocator* const serviceLocator;
  QSet<SmartPointer<IEvaluationReference> > restrictionExpression;

public:

  /**
   * Constructs a new instance of <code>MenuService</code> using a menu
   * manager.
   *
   * @param parent
   *     The parent menu service for this window. This parent must track menu
   *     definitions and the regsitry. Must not be <code>null</code>
   */
  SlaveMenuService(InternalMenuService* parent,
                   IServiceLocator* serviceLocator,
                   const QSet<SmartPointer<IEvaluationReference> >& restrictions);

  void PopulateContributionManager(ContributionManager* mgr, const QString& uri);

  void PopulateContributionManager(ContributionManager* mgr,
                                   const QString& uri, bool recurse);

  SmartPointer<IEvaluationContext> GetCurrentState() const;

  void AddContributionFactory(const SmartPointer<AbstractContributionFactory>& cache);

  void ReleaseContributions(ContributionManager* mgr);

  void RemoveContributionFactory(const SmartPointer<AbstractContributionFactory>& factory);

  void Dispose();

  void AddSourceProvider(const SmartPointer<ISourceProvider>& provider);

  void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider);

  QList<SmartPointer<AbstractContributionFactory> > GetAdditionsForURI(const QUrl& uri);

  void RegisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                           const SmartPointer<Expression>& visibleWhen,
                           QSet<SmartPointer<IEvaluationReference> >& restriction,
                           const QString& identifierID);

  void UnregisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                             QSet<SmartPointer<IEvaluationReference> >& restriction);

  void PopulateContributionManager(
      IServiceLocator* serviceLocatorToUse, const QSet<SmartPointer<IEvaluationReference> >& restriction,
      ContributionManager* mgr, const QString& uri, bool recurse);
};

}

#endif // BERRYSLAVEMENUSERVICE_H
