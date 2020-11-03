/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  void PopulateContributionManager(ContributionManager* mgr, const QString& uri) override;

  void PopulateContributionManager(ContributionManager* mgr,
                                   const QString& uri, bool recurse) override;

  SmartPointer<IEvaluationContext> GetCurrentState() const override;

  void AddContributionFactory(const SmartPointer<AbstractContributionFactory>& cache) override;

  void ReleaseContributions(ContributionManager* mgr) override;

  void RemoveContributionFactory(const SmartPointer<AbstractContributionFactory>& factory) override;

  void Dispose() override;

  void AddSourceProvider(const SmartPointer<ISourceProvider>& provider) override;

  void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider) override;

  QList<SmartPointer<AbstractContributionFactory> > GetAdditionsForURI(const QUrl& uri) override;

  void RegisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                           const SmartPointer<Expression>& visibleWhen,
                           QSet<SmartPointer<IEvaluationReference> >& restriction,
                           const QString& identifierID) override;

  void UnregisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                             QSet<SmartPointer<IEvaluationReference> >& restriction) override;

  void PopulateContributionManager(
      IServiceLocator* serviceLocatorToUse, const QSet<SmartPointer<IEvaluationReference> >& restriction,
      ContributionManager* mgr, const QString& uri, bool recurse) override;
};

}

#endif // BERRYSLAVEMENUSERVICE_H
