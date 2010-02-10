/*=========================================================================
 
Program:   BlueBerry Platform
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

#ifndef BERRYEDITORSITE_H_
#define BERRYEDITORSITE_H_

#include "berryPartSite.h"
#include "berryEditorDescriptor.h"
#include "../berryIEditorPart.h"
#include "berryWorkbenchPage.h"
#include "../berryIEditorSite.h"
#include "../berryIEditorReference.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 * 
 * An editor container manages the services for an editor.
 */
class EditorSite : public PartSite, public IEditorSite {
    /* package */ //static final int PROP_REUSE_EDITOR = -0x101;

private:
  EditorDescriptor::Pointer desc;

    //ListenerList propChangeListeners = new ListenerList(1);

    // SubActionBars ab = null;
    
  /**
     * Initialize the local services.
     */
//    void InitializeDefaultServices() {
//      // Register an implementation of the service appropriate for the 
//      // EditorSite.
//      final IDragAndDropService editorDTService = new EditorSiteDragAndDropServiceImpl();
//      serviceLocator.registerService(IDragAndDropService.class, editorDTService);
//    }
  
protected:
  
 std::string GetInitialScopeId();

public:
  
  berryObjectMacro(EditorSite)
  
    /**
     * Constructs an EditorSite for an editor.
     */
    EditorSite(IEditorReference::Pointer ref, IEditorPart::Pointer editor,
            WorkbenchPage* page, EditorDescriptor::Pointer desc);
  
//    void SetActionBars(SubActionBars bars) {
//        super.setActionBars(bars);
//        
//        if (bars instanceof IActionBars2) {
//            ab = new SubActionBars2((IActionBars2)bars, this);
//        } else {
//            ab = new SubActionBars(bars, this);
//        }
//    }
    
//    void ActivateActionBars(bool forceVisibility) {
//        if (ab != null) {
//            ab.activate(forceVisibility);
//        }
//        super.activateActionBars(forceVisibility);
//    }

//    void DeactivateActionBars(bool forceHide) {
//        if (ab != null) {
//            ab.deactivate(forceHide);
//        }
//        super.deactivateActionBars(forceHide);
//    }
    
    /**
     * Returns the editor action bar contributor for this editor.
     * <p>
     * An action contributor is responsable for the creation of actions.
     * By design, this contributor is used for one or more editors of the same type.
     * Thus, the contributor returned by this method is not owned completely
     * by the editor.  It is shared.
     * </p>
     *
     * @return the editor action bar contributor
     */
//    IEditorActionBarContributor::Pointer GetActionBarContributor() {
//        EditorActionBars bars = (EditorActionBars) getActionBars();
//        if (bars != null) {
//      return bars.getEditorContributor();
//    }
//        
//        return null;
//    }

    /**
     * Returns the extension editor action bar contributor for this editor.
     */
//    IEditorActionBarContributor::Pointer GetExtensionActionBarContributor() {
//        EditorActionBars bars = (EditorActionBars) getActionBars();
//        if (bars != null) {
//      return bars.getExtensionContributor();
//    }
//        
//        return null;
//    }

    /**
     * Returns the editor
     */
    IEditorPart::Pointer GetEditorPart();

    EditorDescriptor::Pointer GetEditorDescriptor();
    
    
//    void registerContextMenu(final MenuManager menuManager,
//            final ISelectionProvider selectionProvider,
//            final boolean includeEditorInput) {
//        registerContextMenu(getId(), menuManager, selectionProvider,
//                includeEditorInput);
//    }
//    
//    void registerContextMenu(final String menuId,
//            final MenuManager menuManager,
//            final ISelectionProvider selectionProvider,
//            final boolean includeEditorInput) {
//        if (menuExtenders == null) {
//            menuExtenders = new ArrayList(1);
//        }
//        
//        PartSite.registerContextMenu(menuId, menuManager, selectionProvider,
//                includeEditorInput, getPart(), menuExtenders);
//    }
};

}

#endif /*BERRYEDITORSITE_H_*/
