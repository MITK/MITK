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

namespace cherry {

void 
PartSite::RegisterContextMenu(const std::string& menuId,
      const MenuManager menuManager,
      const ISelectionProvider selectionProvider,
      bool includeEditorInput, IWorkbenchPart::ConstPtr part,
      const Collection menuExtenders) {
    /*
     * Check to see if the same menu manager and selection provider have
     * already been used. If they have, then we can just add another menu
     * identifier to the existing PopupMenuExtender.
     */
    final Iterator extenderItr = menuExtenders.iterator();
    boolean foundMatch = false;
    while (extenderItr.hasNext()) {
      final PopupMenuExtender existingExtender = (PopupMenuExtender) extenderItr
          .next();
      if (existingExtender.matches(menuManager, selectionProvider, part)) {
        existingExtender.addMenuId(menuId);
        foundMatch = true;
        break;
      }
    }

    if (!foundMatch) {
      menuExtenders.add(new PopupMenuExtender(menuId, menuManager,
          selectionProvider, part, includeEditorInput));
    }
  }

PartSite::PartSite(IWorkbenchPartReference::Ptr ref, IWorkbenchPart::Ptr part,
      IWorkbenchPage::Ptr page) {
    this.partReference = ref;
    this.part = part;
    this.page = page;
    extensionID = "org.eclipse.ui.UnknownID"; //$NON-NLS-1$
    extensionName = "Unknown Name"; //$NON-NLS-1$

    // Initialize the service locator.
    final IServiceLocator parentServiceLocator = page.getWorkbenchWindow();
    IServiceLocatorCreator slc = (IServiceLocatorCreator) parentServiceLocator
        .getService(IServiceLocatorCreator.class);
    this.serviceLocator = (ServiceLocator) slc.createServiceLocator(
        parentServiceLocator, null);

    initializeDefaultServices();
  }

void 
PartSite::InitializeDefaultServices() {
    serviceLocator.registerService(IWorkbenchPartSite.class, this);
    final Expression defaultExpression = new ActivePartExpression(part);

    final IContextService parentContextService = (IContextService) serviceLocator
        .getService(IContextService.class);
    final IContextService contextService = new SlaveContextService(
        parentContextService, defaultExpression);
    serviceLocator.registerService(IContextService.class, contextService);

    final ICommandService parentCommandService = (ICommandService) serviceLocator
        .getService(ICommandService.class);
    final ICommandService commandService = new SlaveCommandService(
        parentCommandService, IServiceScopes.PARTSITE_SCOPE,
        this);
    serviceLocator.registerService(ICommandService.class, commandService);
  }

PartSite::~PartSite() {
    if (menuExtenders != null) {
      HashSet managers = new HashSet(menuExtenders.size());
      for (int i = 0; i < menuExtenders.size(); i++) {
        PopupMenuExtender ext = (PopupMenuExtender) menuExtenders.get(i);
        managers.add(ext.getManager());
        ext.dispose();
      }
      if (managers.size()>0) {
        for (Iterator iterator = managers.iterator(); iterator
            .hasNext();) {
          MenuManager mgr = (MenuManager) iterator.next();
          mgr.dispose();
        }
      }
      menuExtenders = null;
    }

     if (keyBindingService != null) {
      keyBindingService.dispose();
      keyBindingService = null;
     }

    if (progressService != null) {
      progressService.dispose();
      progressService = null;
    }

    if (serviceLocator != null) {
      serviceLocator.dispose();
    }
    part = null;
  }

 
IActionBars 
PartSite::GetActionBars() {
    return actionBars;
  }

std::string
PartSite::GetId() {
    return extensionID;
  }

IWorkbenchPage* 
PartSite::GetPage() {
    return page;
  }

PartPane 
PartSite::GetPane() {
    return ((WorkbenchPartReference) partReference).getPane();
  }

IWorkbenchPart::Ptr 
PartSite::GetPart() {
    return part;
  }

IWorkbenchPartReference::Ptr 
PartSite::GetPartReference() {
    return partReference;
  }

 std::string 
 PartSite::GetPluginId() {
    return pluginID;
  }

std::string 
PartSite::GetRegisteredName() {
    return extensionName;
  }

ISelectionProvider 
PartSite::GetSelectionProvider() {
    return selectionProvider;
  }

Shell 
PartSite::GetShell() {
    PartPane pane = getPane();

    // Compatibility: This method should not be used outside the UI
    // thread... but since this condition
    // was not always in the JavaDoc, we still try to return our best guess
    // about the shell if it is
    // called from the wrong thread.
    Display currentDisplay = Display.getCurrent();
    if (currentDisplay == null
        || currentDisplay != getWorkbenchWindow().getWorkbench()
            .getDisplay()) {
      // Uncomment this to locate places that try to access the shell from
      // a background thread
      // WorkbenchPlugin.log(new Exception("Error:
      // IWorkbenchSite.getShell() was called outside the UI thread. Fix
      // this code.")); //$NON-NLS-1$

      return getWorkbenchWindow().getShell();
    }

    if (pane == null) {
      return getWorkbenchWindow().getShell();
    }

    Shell s = pane.getShell();

    if (s == null) {
      return getWorkbenchWindow().getShell();
    }

    return s;
  }

 IWorkbenchWindow* 
 PartSite::GetWorkbenchWindow() {
    return page.getWorkbenchWindow();
  }

 void 
 PartSite::RegisterContextMenu(const std::string& menuID, 
      MenuManager menuMgr,
      ISelectionProvider selProvider) {
    if (menuExtenders == null) {
      menuExtenders = new ArrayList(1);
    }

    registerContextMenu(menuID, menuMgr, selProvider, true, getPart(),
        menuExtenders);
  }

void 
PartSite::RegisterContextMenu(MenuManager menuMgr,
      ISelectionProvider selProvider) {
    registerContextMenu(getId(), menuMgr, selProvider);
  }

void 
PartSite::GetContextMenuIds(std::vector<std::string>& menuIds) {
    if (menuExtenders == null) {
      return new String[0];
    }
    ArrayList menuIds = new ArrayList(menuExtenders.size());
    for (Iterator iter = menuExtenders.iterator(); iter.hasNext();) {
      final PopupMenuExtender extender = (PopupMenuExtender) iter.next();
      menuIds.addAll(extender.getMenuIds());
    }
    return (String[]) menuIds.toArray(new String[menuIds.size()]);
  }

void 
PartSite::SetActionBars(SubActionBars bars) {
    actionBars = bars;
  }

void 
PartSite::SetConfigurationElement(IConfigurationElement::Ptr configElement) {

    // Get extension ID.
    extensionID = configElement.getAttribute("id"); //$NON-NLS-1$

    // Get plugin ID.
    pluginID = configElement.getNamespace();

    // Get extension name.
    String name = configElement.getAttribute("name"); //$NON-NLS-1$
    if (name != null) {
      extensionName = name;
    }
  }

void 
PartSite::SetPluginId(const std::string& pluginId) {
    this.pluginID = pluginId;
  }

void 
PartSite::SetId(const std::string& id) {
    extensionID = id;
  }

void 
PartSite::SetPart(IWorkbenchPart::Ptr newPart) {
    part = newPart;
  }

void 
PartSite::SetRegisteredName(const std::string& name) {
    extensionName = name;
  }

void 
PartSite::SetSelectionProvider(ISelectionProvider provider) {
    selectionProvider = provider;
  }

  /*
   * @see IWorkbenchPartSite#getKeyBindingService()
   * 
   * TODO deprecated: use IHandlerService instead
   */
IKeyBindingService 
PartSite::GetKeyBindingService() {
    if (keyBindingService == null) {
      keyBindingService = new KeyBindingService(this);

      // TODO why is this here? and it should be using HandlerSubmissions
      // directly..
      if (this instanceof EditorSite) {
        EditorActionBuilder.ExternalContributor contributor = (EditorActionBuilder.ExternalContributor) ((EditorSite) this)
            .getExtensionActionBarContributor();

        if (contributor != null) {
          ActionDescriptor[] actionDescriptors = contributor
              .getExtendedActions();

          if (actionDescriptors != null) {
            for (int i = 0; i < actionDescriptors.length; i++) {
              ActionDescriptor actionDescriptor = actionDescriptors[i];

              if (actionDescriptor != null) {
                IAction action = actionDescriptors[i]
                    .getAction();

                if (action != null
                    && action.getActionDefinitionId() != null) {
                  keyBindingService.registerAction(action);
                }
              }
            }
          }
        }
      }
    }

    return keyBindingService;
  }

std::string 
PartSite::GetInitialScopeId() {
    return null;
  }

Object 
PartSite::GetAdapter(Class adapter) {

    if (IWorkbenchSiteProgressService.class == adapter) {
      return getSiteProgressService();
    }
    
    if (IWorkbenchPartTestable.class == adapter) {
      return new WorkbenchPartTestable(this);
    }

    return Platform.getAdapterManager().getAdapter(this, adapter);
  }

void 
PartSite::ActivateActionBars(bool forceVisibility) {
    if (actionBars != null) {
      actionBars.activate(forceVisibility);
    }
  }

void 
PartSite::DeactivateActionBars(bool forceHide) {
    if (actionBars != null) {
      actionBars.deactivate(forceHide);
    }
  }

WorkbenchSiteProgressService 
PartSite::GetSiteProgressService() {
    if (progressService == null) {
      progressService = new WorkbenchSiteProgressService(this);
    }
    return progressService;
  }

Object 
PartSite::GetService(const Class key) {
    return serviceLocator.getService(key);
  }

bool 
PartSite::HasService(const Class key) {
    return serviceLocator.hasService(key);
  }

std::string 
PartSite::ToString() {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("PartSite(id="); //$NON-NLS-1$
    buffer.append(getId());
    buffer.append(",pluginId="); //$NON-NLS-1$
    buffer.append(getPluginId());
    buffer.append(",registeredName="); //$NON-NLS-1$
    buffer.append(getRegisteredName());
    buffer.append(",hashCode="); //$NON-NLS-1$
    buffer.append(hashCode());
    buffer.append(')');
    return buffer.toString();
  }

}  // namespace cherry
