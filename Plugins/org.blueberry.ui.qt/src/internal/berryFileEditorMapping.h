/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYFILEEDITORMAPPING_H_
#define BERRYFILEEDITORMAPPING_H_

#include "berryIFileEditorMapping.h"

#include "berryEditorDescriptor.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Implementation of IFileEditorMapping.
 *
 */
class FileEditorMapping : public IFileEditorMapping
{

public:
  berryObjectMacro(FileEditorMapping);

private:
  static const QString STAR; // = "*"; //$NON-NLS-1$
  static const QString DOT; // = ".";  //$NON-NLS-1$

  QString name;

  QString extension;

  // Collection of EditorDescriptor, where the first one
  // is considered the default one.
  QList<IEditorDescriptor::Pointer> editors;

  QList<IEditorDescriptor::Pointer> deletedEditors;

  QList<IEditorDescriptor::Pointer> declaredDefaultEditors;

  /**
   * Compare the editor ids from both lists and return true if they
   * are equals.
   */
  bool CompareList(const QList<IEditorDescriptor::Pointer>& l1,
      const QList<IEditorDescriptor::Pointer>& l2) const;

public:

  /**
   *  Create an instance of this class.
   *
   *  @param name java.lang.QString
   *  @param extension java.lang.QString
   */
  FileEditorMapping(const QString& extension, const QString& name = STAR);

  /**
   * Add the given editor to the list of editors registered.
   *
   * @param editor the editor to add
   */
  void AddEditor(EditorDescriptor::Pointer editor);

  /**
   * Clone the receiver.
   */
  //     Object clone() {
  //        try {
  //            FileEditorMapping clone = (FileEditorMapping) super.clone();
  //            clone.editors = (List) ((ArrayList) editors).clone();
  //            return clone;
  //        } catch (CloneNotSupportedException e) {
  //            return null;
  //        }
  //    }

  /* (non-Javadoc)
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool operator==(const Object* obj) const override;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  IEditorDescriptor::Pointer GetDefaultEditor() override;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  QList<IEditorDescriptor::Pointer> GetEditors() const override;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  QList<IEditorDescriptor::Pointer> GetDeletedEditors() const override;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  QString GetExtension() const override;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  //     ImageDescriptor getImageDescriptor() {
  //        IEditorDescriptor editor = getDefaultEditor();
  //        if (editor == null) {
  //            return WorkbenchImages
  //                    .getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //        }
  //        return editor.getImageDescriptor();
  //    }

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  QString GetLabel() const override;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  QString GetName() const override;

  /**
   * Remove the given editor from the set of editors registered.
   *
   * @param editor the editor to remove
   */
  void RemoveEditor(EditorDescriptor::Pointer editor);

  /**
   * Set the default editor registered for file type
   * described by this mapping.
   *
   * @param editor the editor to be set as default
   */
  void SetDefaultEditor(EditorDescriptor::Pointer editor);

  /**
   * Set the collection of all editors (EditorDescriptor)
   * registered for the file type described by this mapping.
   * Typically an editor is registered either through a plugin or explicitly by
   * the user modifying the associations in the preference pages.
   * This modifies the internal list to share the passed list.
   * (hence the clear indication of list in the method name)
   *
   * @param newEditors the new list of associated editors
   */
  void SetEditorsList(const QList<IEditorDescriptor::Pointer>& newEditors);

  /**
   * Set the collection of all editors (EditorDescriptor)
   * formally registered for the file type described by this mapping
   * which have been deleted by the user.
   * This modifies the internal list to share the passed list.
   * (hence the clear indication of list in the method name)
   *
   * @param newDeletedEditors the new list of associated (but deleted) editors
   */
  void SetDeletedEditorsList(
      const QList<IEditorDescriptor::Pointer>& newDeletedEditors);

  /**
   * Set the file's extension.
   *
   * @param extension the file extension for this mapping
   */
  void SetExtension(const QString& extension);

  /**
   * Set the file's name.
   *
   * @param name the file name for this mapping
   */
  void SetName(const QString& name);

  /**
   * Get the editors that have been declared as default. This may be via plugin
   * declarations or the preference page.
   *
   * @return the editors the default editors
   * @since 3.1
   */
  QList<IEditorDescriptor::Pointer> GetDeclaredDefaultEditors();

  /**
   * Return whether the editor is declared default.
   * If this is EditorDescriptor fails the ExpressionsCheck it will always
   * return <code>false</code>, even if it's the original default editor.
   *
   * @param editor the editor to test
   * @return whether the editor is declared default
   * @since 3.1
   */
  bool IsDeclaredDefaultEditor(IEditorDescriptor::Pointer editor);

  /**
   * Set the default editors for this mapping.
   *
   * @param defaultEditors the editors
   * @since 3.1
   */
  void SetDefaultEditors(
      const QList<IEditorDescriptor::Pointer>& defaultEditors);
};

}

#endif /*BERRYFILEEDITORMAPPING_H_*/
