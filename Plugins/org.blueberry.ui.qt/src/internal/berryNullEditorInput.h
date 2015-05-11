/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYNULLEDITORINPUT_H_
#define BERRYNULLEDITORINPUT_H_

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

  berryObjectMacro(NullEditorInput)

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

#endif /*BERRYNULLEDITORINPUT_H_*/
