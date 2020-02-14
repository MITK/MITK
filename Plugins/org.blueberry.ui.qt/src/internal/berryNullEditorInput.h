/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYnullptrEDITORINPUT_H_
#define BERRYnullptrEDITORINPUT_H_

#include "berryIEditorInput.h"
#include "berryEditorReference.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class NullEditorInput : public IEditorInput {

private: EditorReference::Pointer editorReference;

public:

  berryObjectMacro(NullEditorInput);

  NullEditorInput();

  /**
   * Creates a <code>NullEditorInput</code> for the
   * given editor reference.
   *
   * @param editorReference the editor reference
   */
  NullEditorInput(EditorReference::Pointer editorReference);

   bool Exists() const override;

   QIcon GetIcon() const override;

   QString GetName() const override;

   const IPersistableElement* GetPersistable() const override;

   QString GetToolTipText() const override;

   Object* GetAdapter(const QString &adapterType) const override;

    bool operator==(const Object* o) const override;

};

}

#endif /*BERRYnullptrEDITORINPUT_H_*/
