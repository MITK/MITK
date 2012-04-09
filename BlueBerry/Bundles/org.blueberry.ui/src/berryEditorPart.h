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

#ifndef BERRYEDITORPART_H_
#define BERRYEDITORPART_H_

#include <org_blueberry_ui_Export.h>
#include "berryIEditorPart.h"
#include "berryIEditorInput.h"
#include "berryIEditorSite.h"
#include "berryWorkbenchPart.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * Abstract base implementation of all workbench editors.
 * <p>
 * This class should be subclassed by clients wishing to define new editors.
 * The name of the subclass should be given as the <code>"class"</code>
 * attribute in a <code>editor</code> extension contributed to the workbench's
 * editor extension point (named <code>"org.mitk.ui.editors"</code>).
 * For example, the plug-in's XML markup might contain:
 * <pre>
 * &LT;extension point="org.blueberry.ui.editors"&GT;
 *      &LT;editor id="com.example.myplugin.ed"
 *         name="My Editor"
 *         icon="./images/cedit.gif"
 *       extensions="foo"
 *       class="com.example.myplugin.MyFooEditor"
 *       contributorClass="com.example.myplugin.MyFooEditorContributor"
 *      /&GT;
 * &LT;/extension&GT;
 * </pre>
 * where <code>com.example.myplugin.MyEditor</code> is the name of the
 * <code>EditorPart</code> subclass.
 * </p>
 * <p>
 * Subclasses must implement the following methods:
 * <ul>
 *   <li><code>IEditorPart.init</code> - to initialize editor when assigned its site</li>
 *   <li><code>IWorkbenchPart.createPartControl</code> - to create the editor's controls </li>
 *   <li><code>IWorkbenchPart.setFocus</code> - to accept focus</li>
 *   <li><code>IEditorPart.isDirty</code> - to decide whether a significant change has
 *       occurred</li>
 *   <li><code>IEditorPart.doSave</code> - to save contents of editor</li>
 *   <li><code>IEditorPart.doSaveAs</code> - to save contents of editor</li>
 *   <li><code>IEditorPart.isSaveAsAllowed</code> - to control Save As</li>
 * </ul>
 * </p>
 * <p>
 * Subclasses may extend or reimplement the following methods as required:
 * <ul>
 *   <li><code>IExecutableExtension.setInitializationData</code> - extend to provide additional
 *       initialization when editor extension is instantiated</li>
 *   <li><code>IWorkbenchPart.dispose</code> - extend to provide additional cleanup</li>
 *   <li><code>IAdaptable.getAdapter</code> - reimplement to make the editor
 *       adaptable</li>
 * </ul>
 * </p>
 */
class BERRY_UI EditorPart : public WorkbenchPart , public virtual IEditorPart {

  Q_OBJECT
  Q_INTERFACES(berry::IEditorPart)

public:
  berryObjectMacro(EditorPart);

private:

    /**
     * Editor input, or <code>null</code> if none.
     */
    IEditorInput::Pointer editorInput;


protected:

    /**
     * Creates a new workbench editor.
     */
    EditorPart();

    /**
     * Sets the input to this editor.  This method simply updates the internal
     * member variable.
     *
     * <p>Unlike most of the other set methods on this class, this method does
     * not fire a property change. Clients that call this method from a subclass
     * must ensure that they fire an IWorkbenchPartConstants.PROP_INPUT property
     * change after calling this method but before leaving whatever public method
     * they are in. Clients that expose this method as public API must fire
     * the property change within their implementation of setInput.</p>
     *
     * <p>Note that firing a property change may cause listeners to immediately
     * reach back and call methods on this editor. Care should be taken not to
     * fire the property change until the editor has fully updated its internal
     * state to reflect the new input.</p>
     *
     * @param input the editor input
     *
     * @see #setInputWithNotify(IEditorInput)
     */
    virtual void SetInput(IEditorInput::Pointer input) ;

    /**
     * Sets the input to this editor and fires a PROP_INPUT property change if
     * the input has changed.  This is the convenience method implementation.
     *
     * <p>Note that firing a property change may cause other objects to reach back
     * and invoke methods on the editor. Care should be taken not to call this method
     * until the editor has fully updated its internal state to reflect the
     * new input.</p>
     *
     * @since 3.2
     *
     * @param input the editor input
     */
    virtual void SetInputWithNotify(IEditorInput::Pointer input);

    /* (non-Javadoc)
     * @see org.blueberry.ui.part.WorkbenchPart#setContentDescription(java.lang.String)
     */
    virtual void SetContentDescription(const std::string& description);

    /* (non-Javadoc)
     * @see org.blueberry.ui.part.WorkbenchPart#setPartName(java.lang.String)
     */
    virtual void SetPartName(const std::string& partName);

    /**
     * Checks that the given site is valid for this type of part.
     * The site for an editor must be an <code>IEditorSite</code>.
     *
     * @param site the site to check
     * @since 3.1
     */
    void CheckSite(IWorkbenchPartSite::Pointer site);

public:

    /* (non-Javadoc)
     * Saves the contents of this editor.
     * <p>
     * Subclasses must override this method to implement the open-save-close lifecycle
     * for an editor.  For greater details, see <code>IEditorPart</code>
     * </p>
     *
     * @see IEditorPart
     */
  virtual void DoSave(/*IProgressMonitor monitor*/) = 0;

    /* (non-Javadoc)
     * Saves the contents of this editor to another object.
     * <p>
     * Subclasses must override this method to implement the open-save-close lifecycle
     * for an editor.  For greater details, see <code>IEditorPart</code>
     * </p>
     *
     * @see IEditorPart
     */
  virtual void DoSaveAs() = 0;

    /* (non-Javadoc)
     * Method declared on IEditorPart.
     */
  IEditorInput::Pointer GetEditorInput() const;

    /* (non-Javadoc)
     * Method declared on IEditorPart.
     */
  IEditorSite::Pointer GetEditorSite() const;

    /* (non-Javadoc)
     * Gets the title tool tip text of this part.
     *
     * @return the tool tip text
     */
  std::string GetTitleToolTip() const;

    /* (non-Javadoc)
     * Initializes the editor part with a site and input.
     * <p>
     * Subclasses of <code>EditorPart</code> must implement this method.  Within
     * the implementation subclasses should verify that the input type is acceptable
     * and then save the site and input.  Here is sample code:
     * </p>
     * <pre>
     *    if (!(input instanceof IFileEditorInput))
     *      throw new PartInitException("Invalid Input: Must be IFileEditorInput");
     *    setSite(site);
     *    setInput(input);
     * </pre>
     */
  virtual void Init(IEditorSite::Pointer site, IEditorInput::Pointer input) = 0;


    /* (non-Javadoc)
     * Returns whether the contents of this editor have changed since the last save
     * operation.
     * <p>
     * Subclasses must override this method to implement the open-save-close lifecycle
     * for an editor.  For greater details, see <code>IEditorPart</code>
     * </p>
     *
     * @see IEditorPart
     */
  virtual bool IsDirty() const = 0;

    /* (non-Javadoc)
     * Returns whether the "save as" operation is supported by this editor.
     * <p>
     * Subclasses must override this method to implement the open-save-close lifecycle
     * for an editor.  For greater details, see <code>IEditorPart</code>
     * </p>
     *
     * @see IEditorPart
     */
  virtual bool IsSaveAsAllowed() const = 0;

    /* (non-Javadoc)
     * Returns whether the contents of this editor should be saved when the editor
     * is closed.
     * <p>
     * This method returns <code>true</code> if and only if the editor is dirty
     * (<code>isDirty</code>).
     * </p>
     */
  virtual bool IsSaveOnCloseNeeded() const;

};

}

#endif /*BERRYEDITORPART_H_*/
