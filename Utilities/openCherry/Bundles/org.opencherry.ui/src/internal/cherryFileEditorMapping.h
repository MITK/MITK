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

#ifndef CHERRYFILEEDITORMAPPING_H_
#define CHERRYFILEEDITORMAPPING_H_

#include "../cherryIFileEditorMapping.h"

#include "cherryEditorDescriptor.h"

#include <vector>
#include <list>

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 *
 * Implementation of IFileEditorMapping.
 *
 */
class FileEditorMapping : public IFileEditorMapping
{

public:
  cherryClassMacro(FileEditorMapping)

private:
  static const std::string STAR; // = "*"; //$NON-NLS-1$
  static const std::string DOT; // = ".";  //$NON-NLS-1$

  std::string name;

  std::string extension;

  // Collection of EditorDescriptor, where the first one
  // is considered the default one.
  std::list<IEditorDescriptor::Pointer> editors;

  std::list<IEditorDescriptor::Pointer> deletedEditors;

  std::list<IEditorDescriptor::Pointer> declaredDefaultEditors;

  /**
   * Compare the editor ids from both lists and return true if they
   * are equals.
   */
  bool CompareList(const std::list<IEditorDescriptor::Pointer>& l1,
      const std::list<IEditorDescriptor::Pointer>& l2) const;

public:

  /**
   *  Create an instance of this class.
   *
   *  @param name java.lang.std::string
   *  @param extension java.lang.std::string
   */
  FileEditorMapping(const std::string& extension, const std::string& name = STAR);

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
  bool operator==(const Object* obj) const;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  IEditorDescriptor::Pointer GetDefaultEditor();

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  std::list<IEditorDescriptor::Pointer> GetEditors() const;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  std::list<IEditorDescriptor::Pointer> GetDeletedEditors() const;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  std::string GetExtension() const;

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
  std::string GetLabel() const;

  /* (non-Javadoc)
   * Method declared on IFileEditorMapping.
   */
  std::string GetName() const;

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
  void SetEditorsList(const std::list<IEditorDescriptor::Pointer>& newEditors);

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
      const std::list<IEditorDescriptor::Pointer>& newDeletedEditors);

  /**
   * Set the file's extension.
   *
   * @param extension the file extension for this mapping
   */
  void SetExtension(const std::string& extension);

  /**
   * Set the file's name.
   *
   * @param name the file name for this mapping
   */
  void SetName(const std::string& name);

  /**
   * Get the editors that have been declared as default. This may be via plugin
   * declarations or the preference page.
   *
   * @return the editors the default editors
   * @since 3.1
   */
  std::list<IEditorDescriptor::Pointer> GetDeclaredDefaultEditors();

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
      const std::list<IEditorDescriptor::Pointer>& defaultEditors);
};

}

#endif /*CHERRYFILEEDITORMAPPING_H_*/
