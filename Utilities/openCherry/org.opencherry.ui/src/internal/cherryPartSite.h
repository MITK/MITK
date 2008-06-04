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

#ifndef CHERRYPARTSITE_H_
#define CHERRYPARTSITE_H_

#include <org.opencherry.osgi/service/cherryIConfigurationElement.h>

#include "../cherryIWorkbenchPartSite.h"
#include "../cherryIWorkbenchPartReference.h"

namespace cherry {

struct IWorkbenchPart;
struct IWorkbenchPage;
struct IWorkbenchWindow;
struct PartPane;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * <code>PartSite</code> is the general implementation for an
 * <code>IWorkbenchPartSite</code>. A site maintains the context for a part,
 * including the part, its pane, active contributions, selection provider, etc.
 * Together, these components make up the complete behavior for a part as if it
 * was implemented by one person.
 * 
 * The <code>PartSite</code> lifecycle is as follows ..
 * 
 * <ol>
 * <li>a site is constructed </li>
 * <li>a part is constructed and stored in the part </li>
 * <li>the site calls part.init() </li>
 * <li>a pane is constructed and stored in the site </li>
 * <li>the action bars for a part are constructed and stored in the site </li>
 * <li>the pane is added to a presentation </li>
 * <li>the SWT widgets for the pane and part are created </li>
 * <li>the site is activated, causing the actions to become visible </li>
 * </ol>
 */
class PartSite : public virtual IWorkbenchPartSite {

public:
  cherryClassMacro(PartSite);
  
  /**
   * This is a helper method for the register context menu functionality. It
   * is provided so that different implementations of the
   * <code>IWorkbenchPartSite</code> interface don't have to worry about how
   * context menus should work.
   * 
   * @param menuId
   *            the menu id
   * @param menuManager
   *            the menu manager
   * @param selectionProvider
   *            the selection provider
   * @param includeEditorInput
   *            whether editor inputs should be included in the structured
   *            selection when calculating contributions
   * @param part
   *            the part for this site
   * @param menuExtenders
   *            the collection of menu extenders for this site
   * @see IWorkbenchPartSite#registerContextMenu(MenuManager,
   *      ISelectionProvider)
   */
//public: static void RegisterContextMenu(const std::string& menuId,
//      const MenuManager menuManager,
//      const ISelectionProvider selectionProvider,
//      bool includeEditorInput, IWorkbenchPart::ConstPointer part,
//      const Collection menuExtenders);

private:
  
  IWorkbenchPartReference::Pointer partReference;
  SmartPointer<IWorkbenchPart> part;
  SmartPointer<IWorkbenchPage> page;
  std::string extensionID;
  std::string pluginID;
  std::string extensionName;
  //ISelectionProvider selectionProvider;
  //SubActionBars actionBars;
  //KeyBindingService keyBindingService;
  //ArrayList menuExtenders;
  //WorkbenchSiteProgressService progressService;

//protected: const ServiceLocator serviceLocator;

  /**
   * Build the part site.
   * 
   * @param ref
   *            the part reference
   * @param part
   *            the part
   * @param page
   *            the page it belongs to
   */
public: PartSite(IWorkbenchPartReference::Pointer ref, SmartPointer<IWorkbenchPart> part,
      SmartPointer<IWorkbenchPage> page);

  /**
   * Initialize the local services.
   */
private: void InitializeDefaultServices();

  /**
   * Dispose the contributions.
   */
public: ~PartSite();

  /**
   * Returns the action bars for the part. If this part is a view then it has
   * exclusive use of the action bars. If this part is an editor then the
   * action bars are shared among this editor and other editors of the same
   * type.
   */
//public: virtual IActionBars GetActionBars();

  /**
   * Returns the part registry extension ID.
   * 
   * @return the registry extension ID
   */
public: virtual std::string GetId();

  /**
   * Returns the page containing this workbench site's part.
   * 
   * @return the page containing this part
   */
public: virtual SmartPointer<IWorkbenchPage> GetPage();

  /**
   * Gets the part pane.
   */
public: SmartPointer<PartPane> GetPane();

  /**
   * Returns the part.
   */
public: virtual SmartPointer<IWorkbenchPart> GetPart();

  /**
   * Returns the part reference.
   */
public: virtual IWorkbenchPartReference::Pointer GetPartReference();

  /**
   * Returns the part registry plugin ID. It cannot be <code>null</code>.
   * 
   * @return the registry plugin ID
   */
public: virtual std::string GetPluginId();

  /**
   * Returns the registered name for this part.
   */
public: virtual std::string GetRegisteredName();

  /**
   * Returns the selection provider for a part.
   */
//public: virtual ISelectionProvider GetSelectionProvider();


  /**
   * Returns the workbench window containing this part.
   * 
   * @return the workbench window containing this part
   */
public: virtual SmartPointer<IWorkbenchWindow> GetWorkbenchWindow();

  /**
   * Register a popup menu for extension.
   */
//public: virtual void RegisterContextMenu(const std::string& menuID, 
//      MenuManager menuMgr,
//      ISelectionProvider selProvider);

  /**
   * Register a popup menu with the default id for extension.
   */
//public: virtual void RegisterContextMenu(MenuManager menuMgr,
//      ISelectionProvider selProvider);

  // getContextMenuIds() added by Dan Rubel (dan_rubel@instantiations.com)
  /**
   * Get the registered popup menu identifiers
   */
//public: virtual void GetContextMenuIds(std::vector<std::string>& menuIds);

  /**
   * Sets the action bars for the part.
   */
//public: virtual void SetActionBars(SubActionBars bars);

  /**
   * Sets the configuration element for a part.
   */
public: virtual void SetConfigurationElement(IConfigurationElement::Pointer configElement);

protected: virtual void SetPluginId(const std::string& pluginId);

  /**
   * Sets the part registry extension ID.
   * 
   * @param id
   *            the registry extension ID
   */
protected: virtual void SetId(const std::string& id);

  /**
   * Sets the part.
   */
public: virtual void SetPart(SmartPointer<IWorkbenchPart> newPart);

  /**
   * Sets the registered name for this part.
   * 
   * @param name
   *            the registered name
   */
protected: virtual void SetRegisteredName(const std::string& name);

  /**
   * Set the selection provider for a part.
   */
//public: virtual void SetSelectionProvider(ISelectionProvider provider);

  /*
   * @see IWorkbenchPartSite#getKeyBindingService()
   * 
   * TODO deprecated: use IHandlerService instead
   */
//public: virtual  IKeyBindingService GetKeyBindingService();

protected: virtual std::string GetInitialScopeId();

  /**
   * Get an adapter for this type.
   * 
   * @param adapter
   * @return
   */
public: Object::Pointer GetAdapter(const std::type_info& adapter);

//public: virtual void ActivateActionBars(bool forceVisibility);

//public: virtual void DeactivateActionBars(bool forceHide);

  /**
   * Get a progress service for the receiver.
   * 
   * @return WorkbenchSiteProgressService
   */
//public: virtual WorkbenchSiteProgressService GetSiteProgressService();

//public: Object GetService(const Class key);

//public: bool HasService(const Class key);

  /**
   * Prints out the identifier, the plug-in identifier and the registered
   * name. This is for debugging purposes only.
   * 
   * @since 3.2
   */
public: virtual std::string ToString();
};

}  // namespace cherry

#endif /*CHERRYPARTSITE_H_*/
