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

#ifndef CHERRYIEDITORINPUT_H_
#define CHERRYIEDITORINPUT_H_

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>

#include "cherryUiDll.h"

namespace cherry
{

using namespace osgi::framework;

/**
 * \ingroup org_opencherry_ui
 *
 * <code>IEditorInput</code> is a light weight descriptor of editor input,
 * like a file name but more abstract. It is not a model. It is a description of
 * the model source for an <code>IEditorPart</code>.
 * <p>
 * Clients implementing this editor input interface must override
 * <code>Object#operator==(const Object*)</code> to answer true
 * for two inputs that are
 * the same. The <code>IWorbenchPage.openEditor</code> APIs are dependent on
 * this to find an editor with the same input.
 * </p>
 * <p>
 * Clients should extend this interface to declare new types of editor inputs.
 * </p>
 * <p>
 * An editor input is passed to an editor via the <code>IEditorPart.init</code>
 * method. Due to the wide range of valid editor inputs, it is not possible to
 * define generic methods for getting and setting bytes.
 * </p>
 * <p>
 * Editor input must implement the <code>IAdaptable</code> interface;
 * extensions are managed by the platform's adapter manager.
 * </p>
 * <p>
 * Please note that it is important that the editor input be light weight.
 * Within the workbench, the navigation history tends to hold on to editor
 * inputs as a means of reconstructing the editor at a later time. The
 * navigation history can hold on to quite a few inputs (i.e., the default is
 * fifty). The actual data model should probably not be held in the input.
 * </p>
 *
 *
 * @see org.opencherry.ui.IEditorPart
 * @see org.opencherry.ui.IWorkbenchPage#openEditor(IEditorInput, String)
 * @see org.opencherry.ui.IWorkbenchPage#openEditor(IEditorInput, String, boolean)
 */
struct CHERRY_UI IEditorInput : public Object // public IAdaptable
{
  osgiInterfaceMacro(cherry::IEditorInput);

  /**
   * Returns whether the editor input exists.
   * <p>
   * This method is primarily used to determine if an editor input should
   * appear in the "File Most Recently Used" menu. An editor input will appear
   * in the list until the return value of <code>exists</code> becomes
   * <code>false</code> or it drops off the bottom of the list.
   *
   * @return <code>true</code> if the editor input exists;
   *         <code>false</code> otherwise
   */
  virtual bool Exists() const = 0;

  /**
   * Returns the image descriptor for this input.
   *
   * <p>
   * Note: although a null return value has never been permitted from this
   * method, there are many known buggy implementations that return null.
   * Clients that need the image for an editor are advised to use
   * IWorkbenchPart.getImage() instead of IEditorInput.getImageDescriptor(),
   * or to recover from a null return value in a manner that records the ID of
   * the problematic editor input. Implementors that have been returning null
   * from this method should pick some other default return value (such as
   * ImageDescriptor.getMissingImageDescriptor()).
   * </p>
   *
   * @return the image descriptor for this input; may be <code>null</code> if
   * there is no image.
   */
  //virtual ImageDescriptor GetImageDescriptor() = 0;

  /**
   * Returns the name of this editor input for display purposes.
   * <p>
   * For instance, when the input is from a file, the return value would
   * ordinarily be just the file name.
   *
   * @return the name string; never <code>null</code>;
   */
  virtual std::string GetName() const = 0;

  /**
   * Returns an object that can be used to save the state of this editor
   * input.
   *
   * @return the persistable element, or <code>null</code> if this editor
   *         input cannot be persisted
   */
  //virtual IPersistableElement GetPersistable() = 0;

  /**
   * Returns the tool tip text for this editor input. This text is used to
   * differentiate between two input with the same name. For instance,
   * MyClass.java in folder X and MyClass.java in folder Y. The format of the
   * text varies between input types.
   * </p>
   *
   * @return the tool tip text; never <code>null</code>.
   */
  virtual std::string GetToolTipText() const = 0;

  /**
   * Returns true if two editor inputs are the same
   *
   */
  virtual bool operator==(const Object* o) const = 0;
};

}

#endif /*CHERRYIEDITORINPUT_H_*/
