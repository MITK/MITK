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

#ifndef BERRYIEDITORPART_H_
#define BERRYIEDITORPART_H_

#include "berryIWorkbenchPart.h"
#include "berryISaveablePart.h"

namespace berry
{

using namespace osgi::framework;

struct IEditorInput;
struct IEditorSite;

/**
 * \ingroup org_blueberry_ui
 *
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
 * @see org.blueberry.ui.IWorkbenchPage#openEditor(IEditorInput, String)
 * @see org.blueberry.ui.part.EditorPart
 */
struct BERRY_UI IEditorPart : public virtual IWorkbenchPart,
                               public ISaveablePart {

  osgiInterfaceMacro(berry::IEditorPart);

  virtual ~IEditorPart()
  {
  }


  /**
   * The property id for <code>getEditorInput</code>.
   */
  //static const int PROP_INPUT = IWorkbenchPartConstants.PROP_INPUT;


  /**
   * Returns the input for this editor.  If this value changes the part must
   * fire a property listener event with <code>PROP_INPUT</code>.
   *
   * @return the editor input
   */
  virtual SmartPointer<IEditorInput> GetEditorInput() const = 0;

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
  virtual SmartPointer<IEditorSite> GetEditorSite() const = 0;

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

#endif /*BERRYIEDITORPART_H_*/
