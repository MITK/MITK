/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPartSite.h"

#include "berryIWorkbenchPart.h"
#include "berryIWorkbenchPage.h"
#include "berryIWorkbenchWindow.h"
#include "berryPartPane.h"
#include "berryIContributor.h"
#include "berryIServiceScopes.h"

#include "services/berryIServiceFactory.h"

#include "berryIServiceLocatorCreator.h"
#include "berryWorkbenchPartReference.h"
#include "berryWorkbenchLocationService.h"

namespace berry
{

//void
//PartSite::RegisterContextMenu(const QString& menuId,
//      const MenuManager menuManager,
//      const ISelectionProvider selectionProvider,
//      bool includeEditorInput, IWorkbenchPart::ConstPointer part,
//      const Collection menuExtenders) {
//    /*
//     * Check to see if the same menu manager and selection provider have
//     * already been used. If they have, then we can just add another menu
//     * identifier to the existing PopupMenuExtender.
//     */
//    final Iterator extenderItr = menuExtenders.iterator();
//    boolean foundMatch = false;
//    while (extenderItr.hasNext()) {
//      final PopupMenuExtender existingExtender = (PopupMenuExtender) extenderItr
//          .next();
//      if (existingExtender.matches(menuManager, selectionProvider, part)) {
//        existingExtender.addMenuId(menuId);
//        foundMatch = true;
//        break;
//      }
//    }
//
//    if (!foundMatch) {
//      menuExtenders.add(new PopupMenuExtender(menuId, menuManager,
//          selectionProvider, part, includeEditorInput));
//    }
//  }

PartSite::PartSite(IWorkbenchPartReference::Pointer ref,
    IWorkbenchPart::Pointer _part, IWorkbenchPage* _page) :
  partReference(ref), part(_part), page(_page),
  serviceLocatorOwner(new ServiceLocatorOwner(this))
{

  extensionID = "org.blueberry.ui.UnknownID"; //$NON-NLS-1$
  extensionName = "Unknown Name"; //$NON-NLS-1$

  // Initialize the service locator.
  IServiceLocator* parentServiceLocator = page->GetWorkbenchWindow().GetPointer();
  IServiceLocatorCreator* slc = parentServiceLocator->GetService<IServiceLocatorCreator>();
  this->serviceLocator = slc->CreateServiceLocator(parentServiceLocator, nullptr,
                                                   IDisposable::WeakPtr(serviceLocatorOwner)).Cast<ServiceLocator>();

  InitializeDefaultServices();
}

PartSite::~PartSite()
{
//  if (menuExtenders != null)
//  {
//    HashSet managers = new HashSet(menuExtenders.size());
//    for (int i = 0; i < menuExtenders.size(); i++)
//    {
//      PopupMenuExtender ext = (PopupMenuExtender) menuExtenders.get(i);
//      managers.add(ext.getManager());
//      ext.dispose();
//    }
//    if (managers.size()>0)
//    {
//      for (Iterator iterator = managers.iterator(); iterator
//           .hasNext();)
//      {
//        MenuManager mgr = (MenuManager) iterator.next();
//        mgr.dispose();
//      }
//    }
//    menuExtenders = null;
//  }

//  if (keyBindingService != null)
//  {
//    keyBindingService.dispose();
//    keyBindingService = null;
//  }

//  if (progressService != null)
//  {
//    progressService.dispose();
//    progressService = null;
//  }

  if (serviceLocator.IsNotNull())
  {
    serviceLocator->Dispose();
  }
}

PartSite::ServiceLocatorOwner::ServiceLocatorOwner(PartSite* s)
 : site(s)
 {

}

void PartSite::ServiceLocatorOwner::Dispose()
{
  void* control = site->GetPane()->GetControl();
  if (control != nullptr) {
    site->GetPane()->DoHide();
  }
}

void PartSite::InitializeDefaultServices()
{
  workbenchLocationService.reset(
        new WorkbenchLocationService(IServiceScopes::PARTSITE_SCOPE,
                                     GetWorkbenchWindow()->GetWorkbench(),
                                     GetWorkbenchWindow().GetPointer(), this, 2)
        );
  workbenchLocationService->Register();
  serviceLocator->RegisterService(workbenchLocationService.data());
}

//IActionBars
//PartSite::GetActionBars() {
//    return actionBars;
//  }

QString PartSite::GetId() const
{
  return extensionID;
}

IWorkbenchPage::Pointer PartSite::GetPage()
{
  return IWorkbenchPage::Pointer(page);
}

PartPane::Pointer PartSite::GetPane()
{
  return partReference.Lock().Cast<WorkbenchPartReference>()->GetPane();
}

IWorkbenchPart::Pointer PartSite::GetPart()
{
  return IWorkbenchPart::Pointer(part);
}

IWorkbenchPartReference::Pointer PartSite::GetPartReference()
{
  return partReference.Lock();
}

QString PartSite::GetPluginId() const
{
  return pluginID;
}

QString PartSite::GetRegisteredName() const
{
  return extensionName;
}

ISelectionProvider::Pointer
PartSite::GetSelectionProvider()
{
  return selectionProvider;
}

Shell::Pointer PartSite::GetShell()
{
  PartPane::Pointer pane = GetPane();

  if (!pane) return GetWorkbenchWindow()->GetShell();
  return pane->GetShell();
}


IWorkbenchWindow::Pointer PartSite::GetWorkbenchWindow()
{
  return page->GetWorkbenchWindow();
}

// void
// PartSite::RegisterContextMenu(const QString& menuID,
//      MenuManager menuMgr,
//      ISelectionProvider selProvider) {
//    if (menuExtenders == null) {
//      menuExtenders = new ArrayList(1);
//    }
//
//    registerContextMenu(menuID, menuMgr, selProvider, true, getPart(),
//        menuExtenders);
//  }

//void
//PartSite::RegisterContextMenu(MenuManager menuMgr,
//      ISelectionProvider selProvider) {
//    registerContextMenu(getId(), menuMgr, selProvider);
//  }

//void
//PartSite::GetContextMenuIds(QList<QString>& menuIds) {
//    if (menuExtenders == null) {
//      return new String[0];
//    }
//    ArrayList menuIds = new ArrayList(menuExtenders.size());
//    for (Iterator iter = menuExtenders.iterator(); iter.hasNext();) {
//      final PopupMenuExtender extender = (PopupMenuExtender) iter.next();
//      menuIds.addAll(extender.getMenuIds());
//    }
//    return (String[]) menuIds.toArray(new String[menuIds.size()]);
//  }

//void
//PartSite::SetActionBars(SubActionBars bars) {
//    actionBars = bars;
//  }

void PartSite::SetConfigurationElement(
    IConfigurationElement::Pointer configElement)
{

  // Get extension ID.
  extensionID = configElement->GetAttribute("id");

  // Get plugin ID.
  pluginID = configElement->GetContributor()->GetName();

  // Get extension name.
  QString name = configElement->GetAttribute("name");
  if (!name.isEmpty())
  {
    extensionName = name;
  }
}

void PartSite::SetPluginId(const QString& pluginId)
{
  this->pluginID = pluginId;
}

void PartSite::SetId(const QString& id)
{
  extensionID = id;
}

void PartSite::SetPart(IWorkbenchPart::Pointer newPart)
{
  part = newPart;
}

void PartSite::SetRegisteredName(const QString& name)
{
  extensionName = name;
}

void PartSite::SetSelectionProvider(ISelectionProvider::Pointer provider)
{
  selectionProvider = provider;
}

/*
 * @see IWorkbenchPartSite#getKeyBindingService()
 *
 * TODO deprecated: use IHandlerService instead
 */
//IKeyBindingService
//PartSite::GetKeyBindingService() {
//    if (keyBindingService == null) {
//      keyBindingService = new KeyBindingService(this);
//
//      // TODO why is this here? and it should be using HandlerSubmissions
//      // directly..
//      if (this instanceof EditorSite) {
//        EditorActionBuilder.ExternalContributor contributor = (EditorActionBuilder.ExternalContributor) ((EditorSite) this)
//            .getExtensionActionBarContributor();
//
//        if (contributor != null) {
//          ActionDescriptor[] actionDescriptors = contributor
//              .getExtendedActions();
//
//          if (actionDescriptors != null) {
//            for (int i = 0; i < actionDescriptors.length; i++) {
//              ActionDescriptor actionDescriptor = actionDescriptors[i];
//
//              if (actionDescriptor != null) {
//                IAction action = actionDescriptors[i]
//                    .getAction();
//
//                if (action != null
//                    && action.getActionDefinitionId() != null) {
//                  keyBindingService.registerAction(action);
//                }
//              }
//            }
//          }
//        }
//      }
//    }
//
//    return keyBindingService;
//  }

QString PartSite::GetInitialScopeId()
{
  return "";
}

void* PartSite::GetAdapterImpl(const std::type_info& /*adapter*/) const
{

  //    if (IWorkbenchSiteProgressService.class == adapter) {
  //      return getSiteProgressService();
  //    }
  //
  //    if (IWorkbenchPartTestable.class == adapter) {
  //      return new WorkbenchPartTestable(this);
  //    }
  //
  //    return Platform.getAdapterManager().getAdapter(this, adapter);
  return nullptr;
}

//void
//PartSite::ActivateActionBars(bool forceVisibility) {
//    if (actionBars != null) {
//      actionBars.activate(forceVisibility);
//    }
//  }

//void
//PartSite::DeactivateActionBars(bool forceHide) {
//    if (actionBars != null) {
//      actionBars.deactivate(forceHide);
//    }
//  }

//WorkbenchSiteProgressService
//PartSite::GetSiteProgressService() {
//    if (progressService == null) {
//      progressService = new WorkbenchSiteProgressService(this);
//    }
//    return progressService;
//  }

Object*
PartSite::GetService(const QString& api) {
  return serviceLocator->GetService(api);
}

bool
PartSite::HasService(const QString& api) const {
  return serviceLocator->HasService(api);
}

QString PartSite::ToString() const
{
  QString buffer = "PartSite(id=" + this->GetId() + ",pluginId="
      + this->GetPluginId() + ",registeredName=" + this->GetRegisteredName()
      + ")";
  return buffer;
}

} // namespace berry
