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

#ifndef CHERRYIEDITORPART_H_
#define CHERRYIEDITORPART_H_

#include "cherryIWorkbenchPart.h"

namespace cherry
{

struct IEditorInput;
struct IEditorSite;

/**
 * An editor is a visual component within a workbench page. It is
 * typically used to edit or browse a document or input object. The input 
 * is identified using an <code>IEditorInput</code>.  Modifications made 
 * in an editor part follow an open-save-close lifecycle model (in contrast 
 * to a view part, where modifications are saved to the workbench 
 * immediately).
 * <p>
 * An editor is document or input-centric.  Each editor has an input, and only
 * one editor can exist for each editor input within a page.  This policy has 
 * been designed to simplify part management.  
 * </p><p>
 * An editor should be used in place of a view whenever more than one instance
 * of a document type can exist.
 * </p><p>
 * This interface may be implemented directly.  For convenience, a base
 * implementation is defined in <code>EditorPart</code>.
 * </p>
 * <p>
 * An editor part is added to the workbench in two stages:
 * <ol>
 *  <li>An editor extension is contributed to the workbench registry. This
 *    extension defines the extension id, extension class, and the file 
 *    extensions which are supported by the editor.</li>
 *  <li>An editor part based upon the extension is created and added to the
 *    workbench when the user opens a file with one of the supported file
 *    extensions (or some other suitable form of editor input).</li>
 * </ol>
 * </p>
 * <p>
 * All editor parts implement the <code>IAdaptable</code> interface; extensions
 * are managed by the platform's adapter manager.
 * </p>
 *
 * @see org.eclipse.ui.IWorkbenchPage#openEditor(IEditorInput, String)
 * @see org.eclipse.ui.part.EditorPart
 */
struct CHERRY_UI IEditorPart : public virtual IWorkbenchPart
{ //, ISaveablePart {

  cherryClassMacro(IEditorPart)

  virtual ~IEditorPart()
  {
  }

  /**
   * The property id for <code>isDirty</code>.
   */
  //static const int PROP_DIRTY = IWorkbenchPartConstants.PROP_DIRTY;

  /**
   * The property id for <code>getEditorInput</code>.
   */
  //static const int PROP_INPUT = IWorkbenchPartConstants.PROP_INPUT;


  /**
   * Saves the contents of this part.
   * <p>
   * If the save is successful, the part should fire a property changed event 
   * reflecting the new dirty state (<code>PROP_DIRTY</code> property).
   * </p>
   * <p>
   * If the save is cancelled through user action, or for any other reason, the
   * part should invoke <code>setCancelled</code> on the <code>IProgressMonitor</code>
   * to inform the caller.
   * </p>
   * <p>
   * This method is long-running; progress and cancellation are provided
   * by the given progress monitor. 
   * </p>
   *
   * @param monitor the progress monitor
   */
  virtual void DoSave(/*IProgressMonitor monitor*/) = 0;

  /**
   * Saves the contents of this part to another object.
   * <p>
   * Implementors are expected to open a "Save As" dialog where the user will
   * be able to select a new name for the contents. After the selection is made,
   * the contents should be saved to that new name.  During this operation a
   * <code>IProgressMonitor</code> should be used to indicate progress.
   * </p>
   * <p>
   * If the save is successful, the part fires a property changed event 
   * reflecting the new dirty state (<code>PROP_DIRTY</code> property).
   * </p>
   */
  virtual void DoSaveAs() = 0;

  /**
   * Returns whether the contents of this part have changed since the last save
   * operation. If this value changes the part must fire a property listener 
   * event with <code>PROP_DIRTY</code>.
   * <p>
   * <b>Note:</b> this method is called often on a part open or part
   * activation switch, for example by actions to determine their 
   * enabled status.
   * </p>
   *
   * @return <code>true</code> if the contents have been modified and need
   *   saving, and <code>false</code> if they have not changed since the last
   *   save
   */
  virtual bool IsDirty() = 0;

  /**
   * Returns whether the "Save As" operation is supported by this part.
   *
   * @return <code>true</code> if "Save As" is supported, and <code>false</code>
   *  if not supported
   */
  virtual bool IsSaveAsAllowed() = 0;

  /**
   * Returns whether the contents of this part should be saved when the part
   * is closed.
   *
   * @return <code>true</code> if the contents of the part should be saved on
   *   close, and <code>false</code> if the contents are expendable
   */
  virtual bool IsSaveOnCloseNeeded() = 0;

  /**
   * Returns the input for this editor.  If this value changes the part must 
   * fire a property listener event with <code>PROP_INPUT</code>.
   *
   * @return the editor input
   */
  virtual SmartPointer<IEditorInput> GetEditorInput() = 0;

  /**
   * Returns the site for this editor. 
   * This method is equivalent to <code>(IEditorSite) getSite()</code>.
   * <p>  
   * The site can be <code>null</code> while the editor is being initialized. 
   * After the initialization is complete, this value must be non-<code>null</code>
   * for the remainder of the editor's life cycle.
   * </p>
   * 
   * @return the editor site; this value may be <code>null</code> if the editor
   *         has not yet been initialized
   */
  virtual SmartPointer<IEditorSite> GetEditorSite() = 0;

  /**
   * Initializes this editor with the given editor site and input.
   * <p>
   * This method is automatically called shortly after the part is instantiated.
   * It marks the start of the part's lifecycle. The 
   * {@link IWorkbenchPart#dispose IWorkbenchPart.dispose} method will be called 
   * automically at the end of the lifecycle. Clients must not call this method.
   * </p><p>
   * Implementors of this method must examine the editor input object type to
   * determine if it is understood.  If not, the implementor must throw
   * a <code>PartInitException</code>
   * </p>
   * @param site the editor site
   * @param input the editor input
   * @exception PartInitException if this editor was not initialized successfully
   */
  virtual void Init(SmartPointer<IEditorSite> site,
      SmartPointer<IEditorInput> input) = 0;
};

}

#endif /*CHERRYIEDITORPART_H_*/
