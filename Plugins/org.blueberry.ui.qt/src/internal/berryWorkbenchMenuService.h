/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYWORKBENCHMENUSERVICE_H
#define BERRYWORKBENCHMENUSERVICE_H

#include "berryInternalMenuService.h"

#include "berryIPropertyChangeListener.h"

#include <QString>
#include <QSet>

namespace berry {

struct IEvaluationService;
struct IEvaluationReference;
struct IContributionManager;
struct IConfigurationElement;
struct IRegistryChangeEvent;

class AbstractMenuAdditionCacheEntry;
class ContributionRoot;
class ToolBarManager;

/**
 * <p>
 * Provides services related to contributing menu elements to the workbench.
 * </p>
 * <p>
 * This class is only intended for internal use.
 * </p>
 */
class WorkbenchMenuService : public InternalMenuService, private IPropertyChangeListener
{

private:

  friend class ContributionItemUpdater;

  class ManagerPopulationRecord
  {
  public:

    WorkbenchMenuService* wms;
    IServiceLocator* serviceLocatorToUse;
    QSet<SmartPointer<IEvaluationReference> > restriction;
    QString uri;
    bool recurse;

    QHash<SmartPointer<AbstractContributionFactory>, SmartPointer<ContributionRoot> > factoryToItems;

    ManagerPopulationRecord();

    ManagerPopulationRecord(WorkbenchMenuService* wms, IServiceLocator* serviceLocatorToUse,
                            const QSet<SmartPointer<IEvaluationReference> >& restriction, const QString& uri, bool recurse);

    void AddFactoryContribution(const SmartPointer<AbstractContributionFactory>& factory,
                                const SmartPointer<ContributionRoot>& ciList);

    void RemoveFactoryContribution(const SmartPointer<AbstractContributionFactory>& factory);

    SmartPointer<ContributionRoot> GetContributions(const SmartPointer<AbstractContributionFactory>& factory) const;

    /**
     * Delegates back to the workbench to remove -all- the contributions
     * associated with this contribution manager
     */
    void ReleaseContributions();
  };

  static const QString INDEX_AFTER_ADDITIONS_QK; // = "after";
  static const QString INDEX_AFTER_ADDITIONS_QV; // = "additions";
  static const QString PROP_VISIBLE; // = "visible";

  typedef QList<SmartPointer<AbstractContributionFactory> > FactoryListType;

  QHash<QUrl,FactoryListType> uriToFactories;

  QHash<SmartPointer<IContributionItem>,SmartPointer<IEvaluationReference> > evaluationsByItem;

  //QHash activityListenersByItem;

  QSet<IContributionManager*> managersAwaitingUpdates;

  QHash<IContributionManager*,ManagerPopulationRecord> populatedManagers;

  //SmartPointer<IRegistryChangeListener> registryChangeListener;

  /**
   * The class providing persistence for this service.
   */
  //MenuPersistence menuPersistence;

  /**
   * The central authority for determining which menus are visible within this
   * window.
   */
  IEvaluationService* evaluationService;

  /**
   * The service locator into which this service will be inserted.
   */
  IServiceLocator* serviceLocator;

  //SmartPointer<IActivityManagerListener> activityManagerListener;

public:

  berryObjectMacro(berry::WorkbenchMenuService);

  /**
   * Constructs a new instance of <code>MenuService</code> using a menu
   * manager.
   */
  WorkbenchMenuService(IServiceLocator* serviceLocator);

  ~WorkbenchMenuService() override;

  void Dispose() override;

  void AddSourceProvider(const SmartPointer<ISourceProvider>& provider) override;

  void ReadRegistry();

  void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider) override;

  void UpdateManagers();

  FactoryListType GetAdditionsForURI(const QUrl& uri) override;

  /*
   * @see IMenuService#AddContributionFactory(AbstractContributionFactory)
   */
  void AddContributionFactory(const SmartPointer<AbstractContributionFactory>& factory) override;

  /*
   * @see IMenuService#RemoveContributionFactory(AbstractContributionFactory*)
   */
  void RemoveContributionFactory(const SmartPointer<AbstractContributionFactory> &factory) override;

  /*
   * @see org.eclipse.ui.internal.menus.IMenuService#populateMenu(org.eclipse.jface.action.ContributionManager,
   *      org.eclipse.ui.internal.menus.MenuLocationURI)
   */
  void PopulateContributionManager(ContributionManager* mgr, const QString& uri) override;

  void PopulateContributionManager(IServiceLocator* serviceLocatorToUse,
                                   const QSet<SmartPointer<IEvaluationReference> >& restriction,
                                   ContributionManager* mgr,
                                   const QString& uri, bool recurse) override;

  void AddContributionsToManager(IServiceLocator* serviceLocatorToUse,
                                 const QSet<SmartPointer<IEvaluationReference> >& restriction,
                                 ContributionManager *mgr,
                                 const QString& uri, bool recurse,
                                 const QList<SmartPointer<AbstractContributionFactory> >& factories);

  /*
   * @see IMenuService#GetCurrentState()
   */
  SmartPointer<IEvaluationContext> GetCurrentState() const override;

  /*
   * @see IMenuService#RegisterVisibleWhen(IContributionItem, Expression)
   */
  void RegisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                           const SmartPointer<Expression>& visibleWhen,
                           QSet<SmartPointer<IEvaluationReference> >& restriction,
                           const QString& identifierID) override;

  /*
   * @see IMenuService#UnregisterVisibleWhen(IContributionItem)
   */
  void UnregisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                             QSet<SmartPointer<IEvaluationReference> >& restriction) override;

  /*
   * @see IMenuService#ReleaseMenu(ContributionManager)
   */
  void ReleaseContributions(ContributionManager* mgr) override;

  /**
   * Process additions to the menus that occur through ExtensionRegistry changes
   * @param menuAdditions The list of new menu addition extensions to process
   */
  //void HandleDynamicAdditions(const QList<SmartPointer<IConfigurationElement> >& menuAdditions);

  /**
   * Process additions to the menus that occur through ExtensionRegistry changes
   * @param menuRemovals The list of menu addition extensions to remove
   */
  //void HandleDynamicRemovals(const QList<SmartPointer<IConfigurationElement> >& menuRemovals);

  /**
   * @param event
   */
  //void HandleRegistryChanges(const SmartPointer<IRegistryChangeEvent>& event);

  /**
   * @return Returns the menuPersistence.
   */
  //MenuPersistence* GetMenuPersistence();

  /*
   * @see InternalMenuService#PopulateContributionManager(ContributionManager, QString, bool)
   */
  void PopulateContributionManager(ContributionManager* mgr,
                                   const QString& uri, bool recurse) override;

protected:

  /**
   * Removes all cached info for the given manager/factory tuple. This
   * includes unregistering expressions...
   *
   * @param manager The contribution manager owning the contributions
   * @param factory The factory responsible for the contributions
   */
  void RemoveContributionsForFactory(IContributionManager* manager,
                                     const SmartPointer<AbstractContributionFactory> &factory);

  /**
   * @param manager
   */
  void ReleaseContributions(ContributionRoot* items);

private:

  //SmartPointer<IActivityManagerListener> GetActivityManagerListener();

  using IPropertyChangeListener::PropertyChange;
  void PropertyChange(const PropertyChangeEvent::Pointer& event) override;

  IPropertyChangeListener* GetServiceListener();

  //void UpdateTrim(ToolBarManager* mgr);

  bool UpdateToolBar(ToolBarManager* mgr);

  /**
   * Construct an 'id' string from the given URI. The resulting 'id' is the
   * part of the URI not containing the query:
   * <p>
   * i.e. [menu | popup | toolbar]:id
   * </p>
   *
   * @param uri
   *            The URI to construct the id from
   * @return The id
   */
  QString GetIdFromURI(const QUrl& uri);

  /**
   * Return a list of managers that have already been populated and
   * whose URI matches the given one
   * @param factoryId The factoryId to check for
   * @return The list of interested managers
   */
  QList<IContributionManager *> GetManagersFor(const QString& factoryId);

  bool ProcessAdditions(IServiceLocator* serviceLocatorToUse,
                        const QSet<SmartPointer<IEvaluationReference> >& restriction,
                        ContributionManager *mgr,
                        const SmartPointer<AbstractContributionFactory>& cache,
                        QSet<QString> &itemsAdded);

  /**
   * Determines whether the factory should be processed for this manager.
   *
   * @param factory
   *            The factory to be added
   * @param mgr
   *            The contribution manager
   * @return <code>true</code> if the factory to be processed,
   *         <code>false</code> otherwise
   */
  bool ProcessFactory(ContributionManager *mgr,
                      const SmartPointer<AbstractContributionFactory>& factory);

  /**
   * @param items
   */
  void ReleaseCache(ContributionRoot* items);

  int GetInsertionIndex(ContributionManager *mgr, const QString& location);

  /**
   * @param item
   */
  void ReleaseItem(const SmartPointer<IContributionItem>& item, QSet<SmartPointer<IEvaluationReference> >& restriction);

  /**
   * Return whether or not this contribution is programmatic (ie: has a class attribute).
   *
   * @param menuAddition
   * @return whether or not this contribution is programamtic
   */
  bool IsProgramaticContribution(const SmartPointer<IConfigurationElement>& menuAddition) const;

  /**
   * @param ceToRemove
   * @return
   */
  SmartPointer<AbstractMenuAdditionCacheEntry> FindFactory(const SmartPointer<IConfigurationElement>& ceToRemove);

  //void HandleMenuChanges(const SmartPointer<IRegistryChangeEvent>& event);

};

}

#endif // BERRYWORKBENCHMENUSERVICE_H
