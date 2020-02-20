/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPARTSITE_H_
#define BERRYPARTSITE_H_

#include <berryIConfigurationElement.h>

#include "berryServiceLocator.h"

#include "berryIWorkbenchPartSite.h"
#include "berryIWorkbenchPartReference.h"
#include "berryISelectionProvider.h"

namespace berry {

struct IWorkbenchPart;
struct IWorkbenchPage;
struct IWorkbenchWindow;
struct IWorkbenchLocationService;
class  PartPane;
class  Shell;

/**
 * \ingroup org_blueberry_ui_internal
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
class PartSite : public virtual IWorkbenchPartSite
{

public:
  berryObjectMacro(PartSite, IWorkbenchPartSite);

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
//public: static void RegisterContextMenu(const QString& menuId,
//      const MenuManager menuManager,
//      const ISelectionProvider selectionProvider,
//      bool includeEditorInput, IWorkbenchPart::ConstPointer part,
//      const Collection menuExtenders);

private:

  IWorkbenchPartReference::WeakPtr partReference;
  WeakPointer<IWorkbenchPart> part;
  IWorkbenchPage* page;
  QString extensionID;
  QString pluginID;
  QString extensionName;
  ISelectionProvider::Pointer selectionProvider;
  //SubActionBars actionBars;
  //KeyBindingService keyBindingService;
  //ArrayList menuExtenders;
  //WorkbenchSiteProgressService progressService;

  struct ServiceLocatorOwner : public IDisposable
  {
    ServiceLocatorOwner(PartSite* site);

    PartSite* site;

    void Dispose() override;
  };

  ServiceLocatorOwner::Pointer serviceLocatorOwner;

  QScopedPointer<IWorkbenchLocationService, QScopedPointerObjectDeleter> workbenchLocationService;

protected:
  ServiceLocator::Pointer serviceLocator;

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
      IWorkbenchPage* page);

  /**
   * Initialize the local services.
   */
private: void InitializeDefaultServices();

  /**
   * Dispose the contributions.
   */
public: ~PartSite() override;

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
public: QString GetId() const override;

  /**
   * Returns the page containing this workbench site's part.
   *
   * @return the page containing this part
   */
public: SmartPointer<IWorkbenchPage> GetPage() override;

  /**
   * Gets the part pane.
   */
public: SmartPointer<PartPane> GetPane();

  /**
   * Returns the part.
   */
public: SmartPointer<IWorkbenchPart> GetPart() override;

  /**
   * Returns the part reference.
   */
public: virtual IWorkbenchPartReference::Pointer GetPartReference();

  /**
   * Returns the part registry plugin ID. It cannot be <code>null</code>.
   *
   * @return the registry plugin ID
   */
public: QString GetPluginId() const override;

  /**
   * Returns the registered name for this part.
   */
public: QString GetRegisteredName() const override;

  /**
   * Returns the selection provider for a part.
   */
public: ISelectionProvider::Pointer GetSelectionProvider() override;

  /**
   * Returns the shell containing this part.
   *
   * @return the shell containing this part
   */
public: SmartPointer<Shell> GetShell() override;

  /**
   * Returns the workbench window containing this part.
   *
   * @return the workbench window containing this part
   */
public: SmartPointer<IWorkbenchWindow> GetWorkbenchWindow() override;

  /**
   * Register a popup menu for extension.
   */
//public: virtual void RegisterContextMenu(const QString& menuID,
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
//public: virtual void GetContextMenuIds(QList<QString>& menuIds);

  /**
   * Sets the action bars for the part.
   */
//public: virtual void SetActionBars(SubActionBars bars);

  /**
   * Sets the configuration element for a part.
   */
public: virtual void SetConfigurationElement(IConfigurationElement::Pointer configElement);

protected: virtual void SetPluginId(const QString& pluginId);

  /**
   * Sets the part registry extension ID.
   *
   * @param id
   *            the registry extension ID
   */
protected: virtual void SetId(const QString& id);

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
protected: virtual void SetRegisteredName(const QString& name);

  /**
   * Set the selection provider for a part.
   */
public: void SetSelectionProvider(ISelectionProvider::Pointer provider) override;

  /*
   * @see IWorkbenchPartSite#getKeyBindingService()
   *
   * TODO deprecated: use IHandlerService instead
   */
//public: virtual  IKeyBindingService GetKeyBindingService();

protected: virtual QString GetInitialScopeId();

  /**
   * Get an adapter for this type.
   *
   * @param adapter
   * @return
   */
protected: void* GetAdapterImpl(const std::type_info& adapter) const;

//public: virtual void ActivateActionBars(bool forceVisibility);

//public: virtual void DeactivateActionBars(bool forceHide);

  /**
   * Get a progress service for the receiver.
   *
   * @return WorkbenchSiteProgressService
   */
//public: virtual WorkbenchSiteProgressService GetSiteProgressService();

public: Object* GetService(const QString& api) override;

public: bool HasService(const QString& api) const override;

  /**
   * Prints out the identifier, the plug-in identifier and the registered
   * name. This is for debugging purposes only.
   *
   * @since 3.2
   */
public: QString ToString() const override;
};

}  // namespace berry

#endif /*BERRYPARTSITE_H_*/
