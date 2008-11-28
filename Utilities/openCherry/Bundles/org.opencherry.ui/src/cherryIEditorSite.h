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

#ifndef CHERRYIEDITORSITE_H_
#define CHERRYIEDITORSITE_H_

#include "cherryIWorkbenchPartSite.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 * The primary interface between an editor part and the workbench.
 * <p>
 * The workbench exposes its implemention of editor part sites via this 
 * interface, which is not intended to be implemented or extended by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IEditorSite : public virtual IWorkbenchPartSite {
  
  cherryClassMacro(IEditorSite);
  
  virtual ~IEditorSite() {}

    /**
     * Returns the action bar contributor for this editor.
     * <p>
     * An action contributor is responsable for the creation of actions.
     * By design, this contributor is used for one or more editors of the same type.
     * Thus, the contributor returned by this method is not owned completely
     * by the editor - it is shared.
     * </p>
     *
     * @return the editor action bar contributor, or <code>null</code> if none exists
     */
    //virtual IEditorActionBarContributor getActionBarContributor();

    /**
   * Returns the action bars for this part site. Editors of the same type (and
   * in the same window) share the same action bars. Contributions to the
   * action bars are done by the <code>IEditorActionBarContributor</code>.
   * 
   * @return the action bars
   * @since 2.1
   */
    //virtual IActionBars getActionBars();

    /**
     * <p>
     * Registers a pop-up menu with the default id for extension. The default id
     * is defined as the part id.
     * </p>
     * <p>
     * By default, context menus include object contributions based on the
     * editor input for the current editor. It is possible to override this
     * behaviour by calling this method with <code>includeEditorInput</code>
     * as <code>false</code>. This might be desirable for editors that
     * present a localized view of an editor input (e.g., a node in a model
     * editor).
     * </p>
     * <p>
     * For a detailed description of context menu registration see
     * {@link IWorkbenchPartSite#registerContextMenu(MenuManager, ISelectionProvider)}
     * </p>
     * 
     * @param menuManager
     *            the menu manager; must not be <code>null</code>.
     * @param selectionProvider
     *            the selection provider; must not be <code>null</code>.
     * @param includeEditorInput
     *            Whether the editor input should be included when adding object
     *            contributions to this context menu.
     * @see IWorkbenchPartSite#registerContextMenu(MenuManager,
     *      ISelectionProvider)
     * @since 3.1
     */
//    virtual void registerContextMenu(MenuManager menuManager,
//            ISelectionProvider selectionProvider, boolean includeEditorInput);

    /**
     * <p>
     * Registers a pop-up menu with a particular id for extension. This method
     * should only be called if the target part has more than one context menu
     * to register.
     * </p>
     * <p>
     * By default, context menus include object contributions based on the
     * editor input for the current editor. It is possible to override this
     * behaviour by calling this method with <code>includeEditorInput</code>
     * as <code>false</code>. This might be desirable for editors that
     * present a localized view of an editor input (e.g., a node in a model
     * editor).
     * </p>
     * <p>
     * For a detailed description of context menu registration see
     * {@link IWorkbenchPartSite#registerContextMenu(MenuManager, ISelectionProvider)}
     * </p>
     * 
     * @param menuId
     *            the menu id; must not be <code>null</code>.
     * @param menuManager
     *            the menu manager; must not be <code>null</code>.
     * @param selectionProvider
     *            the selection provider; must not be <code>null</code>.
     * @param includeEditorInput
     *            Whether the editor input should be included when adding object
     *            contributions to this context menu.
     * @see IWorkbenchPartSite#registerContextMenu(MenuManager,
     *      ISelectionProvider)
     * @since 3.1
     */
//    virtual void registerContextMenu(String menuId, MenuManager menuManager,
//            ISelectionProvider selectionProvider, boolean includeEditorInput);
};

}

#endif /*CHERRYIEDITORSITE_H_*/
