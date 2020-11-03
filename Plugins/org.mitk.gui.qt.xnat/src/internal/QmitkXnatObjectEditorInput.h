/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKXNATOBJECTEDITORINPUT_H_
#define QMITKXNATOBJECTEDITORINPUT_H_

#include "berryIEditorInput.h"
#include "ctkXnatObject.h"

class QmitkXnatObjectEditorInput : public berry::IEditorInput {

public:
  berryObjectMacro(QmitkXnatObjectEditorInput);

  explicit QmitkXnatObjectEditorInput(ctkXnatObject* object);
  ~QmitkXnatObjectEditorInput();

  /// \brief Returns the kept ctkXnatObject.
  ctkXnatObject* GetXnatObject() const;

  virtual bool Exists() const override;
  virtual QString GetName() const override;
  virtual QString GetToolTipText() const override;
  QIcon GetIcon() const override;
  const berry::IPersistableElement* GetPersistable() const override;
  berry::Object* GetAdapter(const QString& adapterType) const override;

  virtual bool operator==(const berry::Object* o) const override;

private:

  ctkXnatObject* m_Object;
};

#endif /*QMITKXNATOBJECTEDITORINPUT_H_*/
