/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryPartSite.h"

#include "../cherryIWorkbenchPart.h"
#include "../cherryIWorkbenchPage.h"
#include "../cherryIWorkbenchWindow.h"
#include "../cherryPartPane.h"

#include "cherryIServiceLocatorCreator.h"
#include "cherryWorkbenchPartReference.h"

namespace cherry
{

//void
//PartSite::RegisterContextMenu(const std::string& menuId,
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
    IWorkbenchPart::Pointer _part, IWorkbenchPage::Pointer _page) :
  partReference(ref), part(_part), page(_page),
  serviceLocatorOwner(this)
{

  extensionID = "org.opencherry.ui.UnknownID"; //$NON-NLS-1$
  extensionName = "Unknown Name"; //$NON-NLS-1$

  // Initialize the service locator.
  IServiceLocator* parentServiceLocator = page->GetWorkbenchWindow();
  IServiceLocatorCreator::Pointer slc = parentServiceLocator
      ->GetService(IServiceLocatorCreator::GetManifestName()).Cast<IServiceLocatorCreator>();
  this->serviceLocator = dynamic_cast<ServiceLocator*>(slc->CreateServiceLocator(
      parentServiceLocator, 0, &serviceLocatorOwner));

  //initializeDefaultServices();
}

PartSite::~PartSite()
{
  delete serviceLocator;
}

PartSite::ServiceLocatorOwner::ServiceLocatorOwner(PartSite* s)
 : site(s)
 {

}

void PartSite::ServiceLocatorOwner::Dispose()
{
  void* control = site->GetPane()->GetControl();
  if (control != 0) {
    site->GetPane()->DoHide();
  }
}

void PartSite::InitializeDefaultServices()
{
  //    serviceLocator.registerService(IWorkbenchPartSite.class, this);
  //    final Expression defaultExpression = new ActivePartExpression(part);
  //
  //    final IContextService parentContextService = (IContextService) serviceLocator
  //        .getService(IContextService.class);
  //    final IContextService contextService = new SlaveContextService(
  //        parentContextService, defaultExpression);
  //    serviceLocator.registerService(IContextService.class, contextService);
  //
  //    final ICommandService parentCommandService = (ICommandService) serviceLocator
  //        .getService(ICommandService.class);
  //    final ICommandService commandService = new SlaveCommandService(
  //        parentCommandService, IServiceScopes.PARTSITE_SCOPE,
  //        this);
  //    serviceLocator.registerService(ICommandService.class, commandService);
}

//IActionBars
//PartSite::GetActionBars() {
//    return actionBars;
//  }

std::string PartSite::GetId()
{
  return extensionID;
}

IWorkbenchPage::Pointer PartSite::GetPage()
{
  return page;
}

PartPane::Pointer PartSite::GetPane()
{
  return partReference.Cast<WorkbenchPartReference>()->GetPane();
}

IWorkbenchPart::Pointer PartSite::GetPart()
{
  return part;
}

IWorkbenchPartReference::Pointer PartSite::GetPartReference()
{
  return partReference;
}

std::string PartSite::GetPluginId()
{
  return pluginID;
}

std::string PartSite::GetRegisteredName()
{
  return extensionName;
}

ISelectionProvider::Pointer
PartSite::GetSelectionProvider()
{
  return selectionProvider;
}


IWorkbenchWindow::Pointer PartSite::GetWorkbenchWindow()
{
  return page->GetWorkbenchWindow();
}

// void
// PartSite::RegisterContextMenu(const std::string& menuID,
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
//PartSite::GetContextMenuIds(std::vector<std::string>& menuIds) {
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
  configElement->GetAttribute("id", extensionID); //$NON-NLS-1$

  // Get plugin ID.
  pluginID = configElement->GetContributor();

  // Get extension name.
  std::string name;
  configElement->GetAttribute("name", name); //$NON-NLS-1$
  if (name != "")
  {
    extensionName = name;
  }
}

void PartSite::SetPluginId(const std::string& pluginId)
{
  this->pluginID = pluginId;
}

void PartSite::SetId(const std::string& id)
{
  extensionID = id;
}

void PartSite::SetPart(IWorkbenchPart::Pointer newPart)
{
  part = newPart;
}

void PartSite::SetRegisteredName(const std::string& name)
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

std::string PartSite::GetInitialScopeId()
{
  return "";
}

Object::Pointer PartSite::GetAdapter(const std::type_info& /*adapter*/)
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
  return 0;
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

Object::Pointer
PartSite::GetService(const std::string& api) const {
  return serviceLocator->GetService(api);
}

bool
PartSite::HasService(const std::string& api) const {
  return serviceLocator->HasService(api);
}

std::string PartSite::ToString()
{
  std::string buffer = "PartSite(id=" + this->GetId() + ",pluginId="
      + this->GetPluginId() + ",registeredName=" + this->GetRegisteredName()
      + ")";
  return buffer;
}

} // namespace cherry
