/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryWorkbenchMenuService.h"

#include <berryIPropertyChangeListener.h>
#include <berryIServiceLocator.h>
#include <berryIEvaluationService.h>
#include <berryIEvaluationReference.h>
#include <berryIEvaluationContext.h>
#include <berrySafeRunner.h>
#include <berryAbstractContributionFactory.h>
#include <berryObjects.h>
#include <berryMenuUtil.h>
#include <berryWorkbenchActionConstants.h>

#include "berryAbstractGroupMarker.h"
#include "berryAbstractMenuAdditionCacheEntry.h"
#include "berryAlwaysEnabledExpression.h"
#include "berryContributionItem.h"
#include "berryContributionRoot.h"
#include "berryMenuManager.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchWindow.h"

#include <QUrlQuery>

namespace berry {

const QString WorkbenchMenuService::INDEX_AFTER_ADDITIONS_QK = "after";
const QString WorkbenchMenuService::INDEX_AFTER_ADDITIONS_QV = "additions";
const QString WorkbenchMenuService::PROP_VISIBLE = "visible";


/**
 * A combined property and activity listener that updates the visibility of
 * contribution items in the new menu system.
 */
class ContributionItemUpdater : public IPropertyChangeListener //, public IIdentifierListener
{

private:

  const IContributionItem::Pointer item;
  //IIdentifier identifier;
  bool lastExpressionResult;

  WorkbenchMenuService* wms;

  void UpdateVisibility()
  {
    //bool visible = identifier.IsNotNull() ? (identifier->IsEnabled() && lastExpressionResult)
    //                                      : lastExpressionResult;
    bool visible = lastExpressionResult;
    item->SetVisible(visible);

    IContributionManager* parent = nullptr;
    if (ContributionItem::Pointer ci = item.Cast<ContributionItem>())
    {
      parent = ci->GetParent();
    }
    else if (MenuManager::Pointer mm = item.Cast<MenuManager>())
    {
      parent = mm->GetParent();
    }
    if (parent != nullptr)
    {
      parent->MarkDirty();
      wms->managersAwaitingUpdates.insert(parent);
    }
  }

public:

  ContributionItemUpdater(WorkbenchMenuService* wms, const IContributionItem::Pointer& item) //, IIdentifier identifier)
    : item(item), lastExpressionResult(true), wms(wms)
  {
//    if (identifier.IsNotNull())
//    {
//      this->identifier = identifier;
//      this->identifier->AddIdentifierListener(this);
//      UpdateVisibility(); // force initial visibility to fall in line
//      // with activity enablement
//    }
  }

  /**
   * Dispose of this updater
   */
  ~ContributionItemUpdater() override
  {
//    if (identifier.IsNotNull())
//      identifier->RemoveIdentifierListener(this);
  }

  using IPropertyChangeListener::PropertyChange;

  /*
   * @see IPropertyChangeListener#PropertyChange(PropertyChangeEvent)
   */
  void PropertyChange(const PropertyChangeEvent::Pointer& event) override
  {
    if (event->GetProperty() == WorkbenchMenuService::PROP_VISIBLE)
    {
      if (event->GetNewValue().IsNotNull())
      {
        lastExpressionResult = event->GetNewValue();
      }
      else
      {
        lastExpressionResult = false;
      }
      UpdateVisibility();
    }
  }

  /*
   * @see IIdentifierListener#IdentifierChanged(IdentifierEvent)
   */
//  void identifierChanged(IdentifierEvent identifierEvent)
//  {
//    UpdateVisibility();
//  }

};

WorkbenchMenuService::ManagerPopulationRecord::ManagerPopulationRecord()
  : wms(nullptr), serviceLocatorToUse(nullptr), recurse(false)
{
}

WorkbenchMenuService::ManagerPopulationRecord::
ManagerPopulationRecord(WorkbenchMenuService* wms, IServiceLocator* serviceLocatorToUse, const QSet<SmartPointer<IEvaluationReference> >& restriction,
                        const QString& uri, bool recurse)
  : wms(wms), serviceLocatorToUse(serviceLocatorToUse), restriction(restriction),
    uri(uri), recurse(recurse)
{
}

void WorkbenchMenuService::ManagerPopulationRecord::
AddFactoryContribution(const SmartPointer<AbstractContributionFactory>& factory,
                       const SmartPointer<ContributionRoot>& ciList)
{
  // Remove any existing cache info for this factory
  RemoveFactoryContribution(factory);
  // save the new info
  factoryToItems.insert(factory, ciList);
}

void WorkbenchMenuService::ManagerPopulationRecord::
RemoveFactoryContribution(const SmartPointer<AbstractContributionFactory>& factory)
{
  ContributionRoot::Pointer items = factoryToItems.take(factory);
  if (items.IsNotNull())
  {
    wms->ReleaseContributions(items.GetPointer());
  }
}

SmartPointer<ContributionRoot> WorkbenchMenuService::ManagerPopulationRecord::
GetContributions(const SmartPointer<AbstractContributionFactory>& factory) const
{
  if (factoryToItems.contains(factory))
    return factoryToItems[factory];
  return ContributionRoot::Pointer(nullptr);
}

void WorkbenchMenuService::ManagerPopulationRecord::
ReleaseContributions()
{
  foreach (ContributionRoot::Pointer items, factoryToItems.values())
  {
    wms->ReleaseContributions(items.GetPointer());
  }
  factoryToItems.clear();
}

WorkbenchMenuService::WorkbenchMenuService(IServiceLocator* serviceLocator)
  : evaluationService(nullptr), serviceLocator(serviceLocator)
{
  //this.menuPersistence = new MenuPersistence(this);
  evaluationService = serviceLocator->GetService<IEvaluationService>();
  evaluationService->AddServiceListener(GetServiceListener());
//  IWorkbenchLocationService wls = (IWorkbenchLocationService) serviceLocator
//    .getService(IWorkbenchLocationService.class);
//  wls.getWorkbench()
//      .getActivitySupport().getActivityManager()
//      .addActivityManagerListener(getActivityManagerListener());

//  final IExtensionRegistry registry = Platform.getExtensionRegistry();
//  registryChangeListener = new IRegistryChangeListener() {
//    public void registryChanged(final IRegistryChangeEvent event) {
//      final Display display = PlatformUI.getWorkbench().getDisplay();
//      if (display.isDisposed()) {
//        return;
//      }
//      display.syncExec(new Runnable() {
//        public void run() {
//          handleRegistryChanges(event);
//        }
//      });
//    }
//  };
//  registry.addRegistryChangeListener(registryChangeListener);
}

WorkbenchMenuService::~WorkbenchMenuService()
{
  this->Dispose();
}

void WorkbenchMenuService::Dispose()
{
  //menuPersistence.dispose();
//  if (registryChangeListener != null)
//  {
//    final IExtensionRegistry registry = Platform.getExtensionRegistry();
//    registry.removeRegistryChangeListener(registryChangeListener);
//    registryChangeListener = null;
//  }
  foreach (IEvaluationReference::Pointer ref, evaluationsByItem.values())
  {
    evaluationService->RemoveEvaluationListener(ref);
  }
  evaluationsByItem.clear();

  managersAwaitingUpdates.clear();
  evaluationService->RemoveServiceListener(GetServiceListener());

//  if (activityManagerListener != null)
//  {
//    IWorkbenchLocationService wls = (IWorkbenchLocationService) serviceLocator
//        .getService(IWorkbenchLocationService.class);
//    IWorkbench workbench = wls.getWorkbench();
//    if (workbench != null)
//    {
//      workbench.getActivitySupport().getActivityManager()
//          .removeActivityManagerListener(activityManagerListener);
//    }
//  }
}

void WorkbenchMenuService::AddSourceProvider(const SmartPointer<ISourceProvider>& /*provider*/)
{
  // no-op
}

void WorkbenchMenuService::ReadRegistry()
{
  //menuPersistence.read();
}

void WorkbenchMenuService::RemoveSourceProvider(const SmartPointer<ISourceProvider>& /*provider*/)
{
  // no-op
}

void WorkbenchMenuService::UpdateManagers()
{
  QList<IContributionManager*> managers = managersAwaitingUpdates.toList();
  managersAwaitingUpdates.clear();
  foreach (IContributionManager* mgr, managers)
  {
    mgr->Update(true);
//    if (ToolBarManager* tbMgr = dynamic_cast<ToolBarManager*>(mgr))
//    {
//      if (!UpdateToolBar(tbMgr))
//      {
//        //UpdateTrim((ToolBarManager) mgr);
//      }
//    }
//    else
    if (MenuManager* mMgr = dynamic_cast<MenuManager*>(mgr))
    {
      IContributionManager* parent = mMgr->GetParent();
      if (parent != nullptr)
      {
        parent->Update(true);
      }
    }
  }
}

WorkbenchMenuService::FactoryListType WorkbenchMenuService::GetAdditionsForURI(const QUrl& uri)
{
  if (uri.isEmpty())
    return FactoryListType();

  return uriToFactories[GetIdFromURI(uri)];
}

void WorkbenchMenuService::AddContributionFactory(const SmartPointer<AbstractContributionFactory>& factory)
{
  if (factory.IsNull() || factory->GetLocation().isNull())
    return;

  QUrl uri(factory->GetLocation());
  QString factoryId = GetIdFromURI(uri);
  FactoryListType& factories = uriToFactories[factoryId];

  {
//    MenuAdditionCacheEntry::Pointer mace = factory.Cast<MenuAdditionCacheEntry>();
//    if (mace && mace->HasAdditions())
//    {
//      factories.push_front(factory);
//    }
//    else
    {
      factories.push_back(factory);
    }
  }

  // OK, now update any managers that use this uri
  FactoryListType factoryList;
  factoryList.push_back(factory);
  foreach (IContributionManager* mgr, GetManagersFor(factoryId))
  {
    const ManagerPopulationRecord& mpr = populatedManagers[mgr];
    AddContributionsToManager(mpr.serviceLocatorToUse, mpr.restriction,
                              dynamic_cast<ContributionManager*>(mgr), mpr.uri, mpr.recurse, factoryList);
    mgr->Update(true);
  }
}

void WorkbenchMenuService::RemoveContributionFactory(const SmartPointer<AbstractContributionFactory>& factory)
{
  if (factory.IsNull() || factory->GetLocation().isNull())
    return;

  QUrl uri(factory->GetLocation());
  QString factoryId = GetIdFromURI(uri);
  if (uriToFactories.contains(factoryId))
  {
    FactoryListType& factories = uriToFactories[factoryId];
//    // Before we remove the top-level cache we recursively
//    // remove any sub-caches created by this one
//    if (MenuAdditionCacheEntry::Pointer mace = factory.Cast<MenuAdditionCacheEntry>())
//    {
//      QList<AbstractMenuAdditionCacheEntry::Pointer> subCaches = mace->GetSubCaches();
//      foreach (AbstractMenuAdditionCacheEntry::Pointer amace, subCaches)
//      {
//        RemoveContributionFactory(amace);
//      }
//    }
    factories.removeAll(factory);
  }

  // OK, now update any managers that use this uri
  FactoryListType factoryList;
  factoryList.push_back(factory);
  foreach (IContributionManager* mgr, GetManagersFor(factoryId))
  {
    RemoveContributionsForFactory(mgr, factory);
    mgr->Update(true);
  }
}

void WorkbenchMenuService::PopulateContributionManager(ContributionManager* mgr, const QString& uri)
{
  PopulateContributionManager(serviceLocator, QSet<SmartPointer<IEvaluationReference> >(), mgr, uri, true);
}

void WorkbenchMenuService::PopulateContributionManager(IServiceLocator* serviceLocatorToUse,
                                 const QSet<SmartPointer<IEvaluationReference> >& restriction,
                                 ContributionManager* mgr,
                                 const QString& uri, bool recurse)
{
  // Track this attempt to populate the menu, remembering all the parameters
  if (!populatedManagers.contains(mgr))
  {
    populatedManagers.insert(mgr, ManagerPopulationRecord(this, serviceLocatorToUse,
                                                          restriction, uri, recurse));
  }

  QUrl contributionLocation(uri);
  FactoryListType factories = GetAdditionsForURI(contributionLocation);
  AddContributionsToManager(serviceLocatorToUse, restriction, mgr, uri, recurse, factories);
}

void WorkbenchMenuService::AddContributionsToManager(IServiceLocator* serviceLocatorToUse,
                               const QSet<SmartPointer<IEvaluationReference> >& restriction,
                               ContributionManager* mgr,
                               const QString& uri, bool recurse,
                               const QList<SmartPointer<AbstractContributionFactory> >& factories)
{
  QUrl contributionLocation(uri);

  QList<AbstractContributionFactory::Pointer> retryList;
  QSet<QString> itemsAdded;
  foreach (AbstractContributionFactory::Pointer cache, factories)
  {
    if (!ProcessAdditions(serviceLocatorToUse, restriction, mgr,
                          cache, itemsAdded))
    {
      retryList.push_back(cache);
    }
  }

  // OK, iteratively loop through entries whose URI's could not
  // be resolved until we either run out of entries or the list
  // doesn't change size (indicating that the remaining entries
  // can never be resolved).
  bool done = retryList.isEmpty();
  while (!done)
  {
    // Clone the retry list and clear it
    QList<AbstractContributionFactory::Pointer> curRetry = retryList;
    int retryCount = retryList.size();
    retryList.clear();

    // Walk the current list seeing if any entries can now be resolved
    foreach (AbstractContributionFactory::Pointer cache, curRetry)
    {
      if (!ProcessAdditions(serviceLocatorToUse, restriction, mgr,
                            cache, itemsAdded))
      {
        retryList.push_back(cache);
      }
    }

    // We're done if the retryList is now empty (everything done) or
    // if the list hasn't changed at all (no hope)
    done = retryList.isEmpty() || (retryList.size() == retryCount);
  }

  // Now, recurse through any sub-menus
  foreach (IContributionItem::Pointer curItem, mgr->GetItems())
  {
    if (ContributionManager::Pointer cm = curItem.Cast<ContributionManager>())
    {
      QString id = curItem->GetId();
      if (!id.isEmpty() && (recurse || itemsAdded.contains(id)))
      {
        PopulateContributionManager(serviceLocatorToUse,
                                    restriction, cm.GetPointer(),
                                    contributionLocation.scheme() + ":" + id, true);
      }
    }
//    else if (IToolBarContributionItem::Pointer tbci = curItem.Cast<IToolBarContributionItem>())
//    {
//      if (!(tbci->GetId().isEmpty()) && (recurse || itemsAdded.contains(tbci->GetId())))
//      {
//        PopulateContributionManager(serviceLocatorToUse,
//                                    restriction, dynamic_cast<ContributionManager*>(tbci->GetToolBarManager()),
//                                    contributionLocation.scheme() + ":" + tbci->GetId(), true);
//      }
//    }
  }
}

SmartPointer<IEvaluationContext> WorkbenchMenuService::GetCurrentState() const
{
  return evaluationService->GetCurrentState();
}

void WorkbenchMenuService::RegisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                         const SmartPointer<Expression>& visibleWhen,
                         QSet<SmartPointer<IEvaluationReference> >& /*restriction*/,
                         const QString& /*identifierID*/)
{
  if (item.IsNull())
  {
    throw std::invalid_argument("item cannot be null");
  }
  if (visibleWhen.IsNull())
  {
    throw std::invalid_argument("visibleWhen expression cannot be null");
  }

  if (evaluationsByItem.contains(item))
  {
    QString id = item->GetId();
    WorkbenchPlugin::Log(QString("item is already registered: ") + (id.isEmpty() ? QString("no id") : id));
    return;
  }

  // TODO activity support
//  IIdentifier identifier = null;
//  if (identifierID != null) {
//    identifier = PlatformUI.getWorkbench().getActivitySupport()
//        .getActivityManager().getIdentifier(identifierID);
//  }
//  ContributionItemUpdater* listener =
//      new ContributionItemUpdater(item, identifier);

//  if (visibleWhen != AlwaysEnabledExpression::INSTANCE)
//  {
//    IEvaluationReference::Pointer ref = evaluationService->AddEvaluationListener(
//          visibleWhen, listener, PROP_VISIBLE);
//    restriction.insert(ref);
//    evaluationsByItem.insert(item, ref);
//  }
//  activityListenersByItem.put(item, listener);
}

void WorkbenchMenuService::UnregisterVisibleWhen(const SmartPointer<IContributionItem>& item,
                                                 QSet<SmartPointer<IEvaluationReference> >& restriction)
{
  // TODO activity support
//  ContributionItemUpdater identifierListener = (ContributionItemUpdater) activityListenersByItem
//      .remove(item);
//  if (identifierListener != null) {
//    identifierListener.dispose();
//  }

  IEvaluationReference::Pointer ref = evaluationsByItem.take(item);
  if (ref.IsNull())
  {
    return;
  }

  evaluationService->RemoveEvaluationListener(ref);
  restriction.remove(ref);
}

void WorkbenchMenuService::ReleaseContributions(ContributionManager* mgr)
{
  if (mgr == nullptr)
    return;

  // Recursively remove any contributions from sub-menus
  foreach (IContributionItem::Pointer item, mgr->GetItems())
  {
    if (ContributionManager::Pointer cm = item.Cast<ContributionManager>())
    {
      ReleaseContributions(cm.GetPointer());
    }
//    else if (IToolBarContributionItem::Pointer tbci = item.Cast<IToolBarContributionItem>())
//    {
//      ReleaseContributions(tbci->GetToolBarManager());
//    }
  }

  // Now remove any cached information
  if (populatedManagers.contains(mgr))
  {
    populatedManagers[mgr].ReleaseContributions();
    populatedManagers.remove(mgr);
  }
  managersAwaitingUpdates.remove(mgr);
}

//void WorkbenchMenuService::HandleDynamicAdditions(const QList<SmartPointer<IConfigurationElement> >& menuAdditions)
//{
//  for (Iterator additionsIter = menuAdditions.iterator(); additionsIter.hasNext();) {
//    AbstractContributionFactory newFactory = null;
//    final IConfigurationElement menuAddition = (IConfigurationElement) additionsIter.next();
//    if (isProgramaticContribution(menuAddition))
//      newFactory = new ProxyMenuAdditionCacheEntry(
//          menuAddition
//              .getAttribute(IWorkbenchRegistryConstants.TAG_LOCATION_URI),
//          menuAddition.getNamespaceIdentifier(), menuAddition);

//    else
//      newFactory = new MenuAdditionCacheEntry(
//          this,
//          menuAddition,
//          menuAddition
//              .getAttribute(IWorkbenchRegistryConstants.TAG_LOCATION_URI),
//          menuAddition.getNamespaceIdentifier());

//    if (newFactory != null)
//      addContributionFactory(newFactory);
//  }
//}

//void WorkbenchMenuService::HandleDynamicRemovals(const QList<SmartPointer<IConfigurationElement> >& menuRemovals)
//{
//  for (Iterator additionsIter = menuRemovals.iterator(); additionsIter.hasNext();) {
//    IConfigurationElement ceToRemove = (IConfigurationElement) additionsIter.next();
//    AbstractMenuAdditionCacheEntry factoryToRemove = findFactory(ceToRemove);
//    removeContributionFactory(factoryToRemove);
//  }
//}

//void WorkbenchMenuService::HandleRegistryChanges(const SmartPointer<IRegistryChangeEvent>& event)
//{
//  // HACK!! determine if this is an addition or deletion from the first delta
//  IExtensionDelta[] deltas = event.getExtensionDeltas();
//  if (deltas.length == 0)
//    return;
//  boolean isAddition = deltas[0].getKind() == IExtensionDelta.ADDED;

//  // access all the necessary service persistence handlers
//  HandlerService handlerSvc = (HandlerService) serviceLocator.getService(IHandlerService.class);
//  HandlerPersistence handlerPersistence = handlerSvc.getHandlerPersistence();

//  CommandService cmdSvc = (CommandService) serviceLocator.getService(ICommandService.class);
//  CommandPersistence cmdPersistence = cmdSvc.getCommandPersistence();

//  BindingService bindingSvc = (BindingService) serviceLocator.getService(IBindingService.class);
//  BindingPersistence bindingPersistence = bindingSvc.getBindingPersistence();

//  boolean needsUpdate = false;

//  // determine order from the type of delta
//  if (isAddition) {
//    // additions order: Commands, Handlers, Bindings, Menus
//    if (cmdPersistence.commandsNeedUpdating(event)) {
//      cmdPersistence.reRead();
//      needsUpdate = true;
//    }
//    if (handlerPersistence.handlersNeedUpdating(event)) {
//      handlerPersistence.reRead();
//      needsUpdate = true;
//    }
//    if (bindingPersistence.bindingsNeedUpdating(event)) {
//      bindingPersistence.reRead();
//      needsUpdate = true;
//    }
//    if (menuPersistence.menusNeedUpdating(event)) {
//      handleMenuChanges(event);
//      needsUpdate = true;
//    }
//  }
//  else {
//    // Removal order: Menus, Bindings, Handlers, Commands
//    if (menuPersistence.menusNeedUpdating(event)) {
//      handleMenuChanges(event);
//      needsUpdate = true;
//    }
//    if (bindingPersistence.bindingsNeedUpdating(event)) {
//      bindingPersistence.reRead();
//      needsUpdate = true;
//    }
//    if (handlerPersistence.handlersNeedUpdating(event)) {
//      final IExtensionDelta[] handlerDeltas = event.getExtensionDeltas(
//          PlatformUI.PLUGIN_ID, IWorkbenchRegistryConstants.PL_HANDLERS);
//      for (int i = 0; i < handlerDeltas.length; i++) {
//        IConfigurationElement[] ices = handlerDeltas[i].getExtension().getConfigurationElements();
//        HandlerProxy.updateStaleCEs(ices);
//      }

//      handlerPersistence.reRead();
//      needsUpdate = true;
//    }
//    if (cmdPersistence.commandsNeedUpdating(event)) {
//      cmdPersistence.reRead();
//      needsUpdate = true;
//    }
//  }

//  if (needsUpdate) {
//    ContributionManager[] managers = (ContributionManager[]) populatedManagers
//        .keySet().toArray(
//            new ContributionManager[populatedManagers.keySet()
//                .size()]);
//    for (int i = 0; i < managers.length; i++) {
//      ContributionManager mgr = managers[i];
//      mgr.update(false);
//    }
//  }
//}

//MenuPersistence* WorkbenchMenuService::GetMenuPersistence()
//{
//  return menuPersistence;
//}

void WorkbenchMenuService::PopulateContributionManager(ContributionManager* mgr,
                                 const QString& uri, bool recurse)
{
  PopulateContributionManager(serviceLocator, QSet<IEvaluationReference::Pointer>(), mgr, uri, recurse);
}

void WorkbenchMenuService::RemoveContributionsForFactory(IContributionManager* manager,
                                                         const SmartPointer<AbstractContributionFactory>& factory)
{
  populatedManagers[manager].RemoveFactoryContribution(factory); // automatically cleans its caches
}

void WorkbenchMenuService::ReleaseContributions(ContributionRoot* items)
{
  ContributionManager* mgr = items->GetManager();
  foreach(IContributionItem::Pointer item, items->GetItems())
  {
    ReleaseItem(item, items->restriction);
    mgr->Remove(item);
  }
  ReleaseCache(items);
}

void WorkbenchMenuService::PropertyChange(const PropertyChangeEvent::Pointer& event)
{
  if (event->GetProperty() == IEvaluationService::PROP_NOTIFYING)
  {
    if (!(event->GetNewValue().Cast<ObjectBool>()->GetValue()))
    {
      // if it's false, the evaluation service has
      // finished with its latest round of updates
      this->UpdateManagers();
    }
  }
}

//SmartPointer<IActivityManagerListener> WorkbenchMenuService::GetActivityManagerListener()
//{
//  if (activityManagerListener == null) {
//    activityManagerListener = new IActivityManagerListener() {

//      public void activityManagerChanged(
//          ActivityManagerEvent activityManagerEvent) {
//        if (activityManagerEvent.haveEnabledActivityIdsChanged()) {
//          updateManagers(); // called after all identifiers have
//          // been update - now update the
//          // managers
//        }

//      }
//    };
//  }
//  return activityManagerListener;
//}

IPropertyChangeListener* WorkbenchMenuService::GetServiceListener()
{
  return this;
}

//void WorkbenchMenuService::UpdateTrim(ToolBarManager* mgr)
//{
//  Control control = mgr.getControl();
//  if (control == null || control.isDisposed()) {
//    return;
//  }
//  LayoutUtil.resize(control);
//}

bool WorkbenchMenuService::UpdateToolBar(ToolBarManager* /*mgr*/)
{
//  QList<IWorkbenchWindow::Pointer> windows = PlatformUI::GetWorkbench()->GetWorkbenchWindows();
//  QList<IWorkbenchWindow::Pointer>::iterator wend = windows.end();
//  for (QList<IWorkbenchWindow::Pointer>::iterator i = windows.begin(); i != wend; ++i)
//  {
//    WorkbenchWindow::Pointer window = i->Cast<WorkbenchWindow>();
//    IToolBarManager* tb = window->GetToolBarManager();
//    if (tb != 0)
//    {
//      foreach (IContributionItem::Pointer item, tb->GetItems())
//      {
//        if (ToolBarContributionItem::Pointer tbci = item.Cast<ToolBarContributionItem>())
//        {
//          IToolBarManager* tbm = tbci->GetToolBarManager();
//          if (mgr == tbm)
//          {
//            tb->Update(true);
//            return true;
//          }
//        }
//      }
//    }
//  }
  return false;
}

QString WorkbenchMenuService::GetIdFromURI(const QUrl& uri)
{
  return uri.scheme() + ":" + uri.path();
}

QList<IContributionManager*> WorkbenchMenuService::GetManagersFor(const QString& factoryId)
{
  QList<IContributionManager*> mgrs;

  QHashIterator<IContributionManager*,ManagerPopulationRecord> mgrIter(populatedManagers);
  while(mgrIter.hasNext())
  {
    if (factoryId == mgrIter.value().uri)
    {
      mgrs.push_back(mgrIter.key());
    }
  }

  return mgrs;
}

bool WorkbenchMenuService::ProcessAdditions(IServiceLocator* serviceLocatorToUse,
                      const QSet<SmartPointer<IEvaluationReference> >& restriction,
                      ContributionManager* mgr,
                      const AbstractContributionFactory::Pointer& cache,
                      QSet<QString>& itemsAdded)
{

  if (!ProcessFactory(mgr, cache))
    return true;

  const int idx = GetInsertionIndex(mgr, cache->GetLocation());
  if (idx == -1)
    return false; // can't process (yet)

  struct _SafeRunnable : public ISafeRunnable
  {
    WorkbenchMenuService* wms;
    int insertionIndex;
    IServiceLocator* serviceLocatorToUse;
    QSet<SmartPointer<IEvaluationReference> > restriction;
    ContributionManager* mgr;
    AbstractContributionFactory::Pointer cache;
    QSet<QString>& itemsAdded;

    _SafeRunnable(WorkbenchMenuService* wms, int idx, IServiceLocator* serviceLocatorToUse,
                  const QSet<SmartPointer<IEvaluationReference> >& restriction,
                  ContributionManager* mgr, AbstractContributionFactory::Pointer cache,
                  QSet<QString>& itemsAdded)
      : wms(wms), insertionIndex(idx), serviceLocatorToUse(serviceLocatorToUse),
        restriction(restriction), mgr(mgr), cache(cache), itemsAdded(itemsAdded)
    {}

    void HandleException(const ctkException&) override
    {}

    void Run() override
    {
      // Get the additions
      ContributionRoot::Pointer ciList(new ContributionRoot(wms, restriction,
                                                            mgr, cache.GetPointer()));

      cache->CreateContributionItems(serviceLocatorToUse, ciList);

      // If we have any then add them at the correct location
      if (!ciList->GetItems().isEmpty())
      {
        // Cache the items for future cleanup
        ManagerPopulationRecord& mpr = wms->populatedManagers[mgr];
        ContributionRoot::Pointer contributions = mpr.GetContributions(cache);
        if (contributions.IsNotNull())
        {
          // Existing contributions in the mgr will be released.
          // Adjust the insertionIndex
          foreach (IContributionItem::Pointer item, contributions->GetItems())
          {
            if (item == mgr->Find(item->GetId()))
              insertionIndex--;
          }
        }

        mpr.AddFactoryContribution(cache, ciList);

        foreach (IContributionItem::Pointer ici, ciList->GetItems())
        {
          if ((ici.Cast<ContributionManager>() ||
               //ici.Cast<IToolBarContributionItem> ||
               ici.Cast<AbstractGroupMarker>())
              && !(ici->GetId().isEmpty()))
          {
            IContributionItem::Pointer foundIci = mgr->Find(ici->GetId());
            // really, this is a very specific scenario that
            // allows merging but, if it is a contribution manager that also
            // contains items, then we would be throwing stuff away.
            if (ContributionManager::Pointer cm = foundIci.Cast<ContributionManager>())
            {
              if (cm->GetSize() > 0)
              {
//                IStatus status = new Status(
//                    IStatus.WARNING,
//                    WorkbenchPlugin.PI_WORKBENCH,
//                    "Menu contribution id collision: "
//                        + ici.getId());
//                StatusManager.getManager().handle(status);
                BERRY_WARN << "Menu contribution id collision: " << ici->GetId().toStdString();
              }
              continue;
            }
//            else if (IToolBarContributionItem::Pointer tbci = foundIci.Cast<IToolBarContributionItem>())
//            {
//              IToolBarManager* toolBarManager = tbci->GetToolBarManager();
//              if (ContributionManager::Pointer tbcm = dynamic_cast<ContributionManager*>(toolBarManager)
//                  && tbcm->GetSize() > 0)
//              {
////                IStatus status = new Status(
////                    IStatus.WARNING,
////                    WorkbenchPlugin.PI_WORKBENCH,
////                    "Toolbar contribution id collision: " //$NON-NLS-1$
////                        + ici.getId());
////                StatusManager.getManager().handle(status);
//                BERRY_WARN << "Toolbar contribution id collision: " << ici->GetId().toStdString();
//              }
//              continue;
//            }
            else if (foundIci.Cast<AbstractGroupMarker>())
            {
              continue;
            }
          }
          const int oldSize = mgr->GetSize();
          mgr->Insert(insertionIndex, ici);
          if (!ici->GetId().isEmpty())
          {
            itemsAdded.insert(ici->GetId());
          }
          if (mgr->GetSize() > oldSize)
            insertionIndex++;
        }
      }
    }
  };

  ISafeRunnable::Pointer run (new _SafeRunnable(this, idx, serviceLocatorToUse, restriction,
                                                mgr, cache, itemsAdded));
  SafeRunner::Run(run);

  return true;
}

bool WorkbenchMenuService::ProcessFactory(ContributionManager* mgr,
                                          const AbstractContributionFactory::Pointer& factory)
{
  QUrl uri(factory->GetLocation());
  if (MenuUtil::ANY_POPUP == (uri.scheme() + ':' + uri.path()))
  {
    // its any popup. check whether manager has additions
    if (mgr->IndexOf(WorkbenchActionConstants::MB_ADDITIONS) == -1)
    {
//      // menu has no additions. Add only if allPopups = true
//      if (MenuAdditionCacheEntry::Pointer menuEntry = factory.Cast<MenuAdditionCacheEntry>())
//      {
//        return menuEntry->ContributeToAllPopups();
//      }
    }
  }

  return true;
}

void WorkbenchMenuService::ReleaseCache(ContributionRoot* items)
{
  items->Release();
}

int WorkbenchMenuService::GetInsertionIndex(ContributionManager* mgr, const QString& location)
{
  QUrl uri(location);
  QUrlQuery query(uri);
  QList<QPair<QString,QString> > queryParts = query.queryItems();
  bool indexAfterAdditions = query.queryItemValue(INDEX_AFTER_ADDITIONS_QK) == INDEX_AFTER_ADDITIONS_QV;
  int additionsIndex = -1;

  // No Query means 'after=additions' (if there) or
  // the end of the menu
  if (queryParts.isEmpty() || indexAfterAdditions)
  {
    additionsIndex = mgr->IndexOf(WorkbenchActionConstants::MB_ADDITIONS);
    if (additionsIndex == -1)
      additionsIndex = mgr->GetItems().size();
    else
      ++additionsIndex;
  }
  else
  {
    // Should be in the form "[before|after|endof]=id"
    if (queryParts.size() > 0 && !(queryParts[0].second.isEmpty()))
    {
      QString modifier = queryParts[0].first;
      QString id = queryParts[0].second;
      additionsIndex = mgr->IndexOf(id);
      if (additionsIndex != -1)
      {
        if (MenuUtil::QUERY_BEFORE == modifier)
        {
          // this is OK, the additionsIndex will either be correct
          // or -1 (which is a no-op)
        }
        else if (MenuUtil::QUERY_AFTER == modifier)
        {
          additionsIndex++;
        }
        else if (MenuUtil::QUERY_ENDOF == modifier)
        {
          // OK, this one is exciting
          QList<IContributionItem::Pointer> items = mgr->GetItems();
          for (additionsIndex++; additionsIndex < items.size(); additionsIndex++)
          {
            if (items[additionsIndex]->IsGroupMarker())
            {
              break;
            }
          }
        }
      }
    }
  }

  return additionsIndex;
}

void WorkbenchMenuService::ReleaseItem(const SmartPointer<IContributionItem>& item, QSet<SmartPointer<IEvaluationReference> >& restriction)
{
  UnregisterVisibleWhen(item, restriction);
  if (ContributionManager::Pointer cm = item.Cast<ContributionManager>())
  {
    ReleaseContributions(cm.GetPointer());
  }
//  else if (IToolBarContributionItem::Pointer tbci = item.Cast<IToolBarContributionItem>())
//  {
//    ReleaseContributions(dynamic_cast<ContributionManager*>(tbci->GetToolBarManager()));
//  }
}

bool WorkbenchMenuService::IsProgramaticContribution(const SmartPointer<IConfigurationElement>& menuAddition) const
{
  return !menuAddition->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS).isEmpty();
}

SmartPointer<AbstractMenuAdditionCacheEntry> WorkbenchMenuService::FindFactory(const SmartPointer<IConfigurationElement>& ceToRemove)
{
  QUrl uri = ceToRemove->GetAttribute(WorkbenchRegistryConstants::TAG_LOCATION_URI);
  FactoryListType factories = GetAdditionsForURI(uri);
  foreach (AbstractContributionFactory::Pointer factory, GetAdditionsForURI(uri))
  {
    if (AbstractMenuAdditionCacheEntry::Pointer mace = factory.Cast<AbstractMenuAdditionCacheEntry>())
    {
      if (mace->GetConfigElement() == ceToRemove)
        return mace;
    }
  }
  return AbstractMenuAdditionCacheEntry::Pointer(nullptr);
}

//void WorkbenchMenuService::HandleMenuChanges(const SmartPointer<IRegistryChangeEvent>& event)
//{
//  final IExtensionDelta[] menuDeltas = event.getExtensionDeltas(
//      PlatformUI.PLUGIN_ID, IWorkbenchRegistryConstants.PL_MENUS);
//  final List menuAdditions = new ArrayList();
//  final List menuRemovals = new ArrayList();
//  for (int i = 0; i < menuDeltas.length; i++) {
//    IConfigurationElement[] ices = menuDeltas[i].getExtension().getConfigurationElements();

//    for (int j = 0; j < ices.length; j++) {
//      if (IWorkbenchRegistryConstants.PL_MENU_CONTRIBUTION.equals(ices[j].getName())) {
//        if (menuDeltas[i].getKind() == IExtensionDelta.ADDED)
//          menuAdditions.add(ices[j]);
//        else
//          menuRemovals.add(ices[j]);
//      }
//    }
//  }

//  // Handle additions
//  if (menuAdditions.size() > 0) {
//    handleDynamicAdditions(menuAdditions);
//  }

//  // Handle Removals
//  if (menuRemovals.size() > 0) {
//    handleDynamicRemovals(menuRemovals);
//  }
//}

}
