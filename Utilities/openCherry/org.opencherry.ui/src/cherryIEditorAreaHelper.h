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

#ifndef CHERRYIEDITORAREAHELPER_H_
#define CHERRYIEDITORAREAHELPER_H_

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 */
struct IEditorAreaHelper {

    /**
     * Closes an editor.   
     *
     * @param part the editor to close
     */
    virtual void CloseEditor(IEditorReference::Pointer ref) = 0;

    /**
     * Closes an editor.   
     *
     * @param part the editor to close
     */
    virtual void CloseEditor(IEditorPart::Pointer part) = 0;


    /**
     * Returns the active editor in this perspective.  If the editors appear
     * in a workbook this will be the visible editor.  If the editors are
     * scattered around the workbench this will be the most recent editor
     * to hold focus.
     *
     * @return the active editor, or <code>null</code> if no editor is active
     */
    virtual IEditorReference::Pointer GetVisibleEditor() = 0;


    /**
     * Main entry point for adding an editor. Adds the editor to the layout in the given
     * stack, and notifies the workbench page when done.
     * 
     * @param ref editor to add
     */
    virtual void AddEditor(IEditorReference::Pointer ref) = 0;
    

    /**
     * Brings an editor to the front and optionally gives it focus.
     *
     * @param part the editor to make visible
     * @param setFocus whether to give the editor focus
     * @return true if the visible editor was changed, false if not.
     */
    virtual bool SetVisibleEditor(IEditorReference::Pointer ref, bool setFocus) = 0;

    
    virtual std::vector<IEditorReference::Pointer> GetEditors() = 0;

};

}

#endif /*CHERRYIEDITORAREAHELPER_H_*/
