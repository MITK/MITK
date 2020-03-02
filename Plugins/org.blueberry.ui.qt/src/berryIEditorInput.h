/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIEDITORINPUT_H_
#define BERRYIEDITORINPUT_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <berryIAdaptable.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

struct IPersistableElement;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * <code>IEditorInput</code> is a light weight descriptor of editor input,
 * like a file name but more abstract. It is not a model. It is a description of
 * the model source for an <code>IEditorPart</code>.
 * <p>
 * Clients implementing this editor input interface must override
 * <code>Object#operator==(const Object*)</code> to answer true
 * for two inputs that are
 * the same. The <code>IWorkbenchPage.openEditor</code> APIs are dependent on
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
 * @see org.blueberry.ui.IEditorPart
 * @see org.blueberry.ui.IWorkbenchPage#openEditor(IEditorInput, String)
 * @see org.blueberry.ui.IWorkbenchPage#openEditor(IEditorInput, String, boolean)
 */
struct BERRY_UI_QT IEditorInput : public virtual Object, public virtual IAdaptable
{
  berryObjectMacro(berry::IEditorInput);

  ~IEditorInput() override;

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
   * Returns the icon for this input.
   *
   * @return the icon for this input; may be null icon if
   * there is no image.
   */
  virtual QIcon GetIcon() const = 0;

  /**
   * Returns the name of this editor input for display purposes.
   * <p>
   * For instance, when the input is from a file, the return value would
   * ordinarily be just the file name.
   *
   * @return the name string; never <code>null</code>;
   */
  virtual QString GetName() const = 0;

  /**
   * Returns an object that can be used to save the state of this editor
   * input.
   *
   * @return the persistable element, or <code>null</code> if this editor
   *         input cannot be persisted
   */
  virtual const IPersistableElement* GetPersistable() const = 0;

  /**
   * Returns the tool tip text for this editor input. This text is used to
   * differentiate between two input with the same name. For instance,
   * MyClass.java in folder X and MyClass.java in folder Y. The format of the
   * text varies between input types.
   * </p>
   *
   * @return the tool tip text; never <code>null</code>.
   */
  virtual QString GetToolTipText() const = 0;

  /**
   * Returns true if two editor inputs are the same
   *
   */
  bool operator==(const Object* o) const override = 0;
};

}

#endif /*BERRYIEDITORINPUT_H_*/
