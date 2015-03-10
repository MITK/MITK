/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKXNATOBJECTEDITORINPUT_H_
#define QMITKXNATOBJECTEDITORINPUT_H_

#include "berryIEditorInput.h"
#include "ctkXnatObject.h"

class QmitkXnatObjectEditorInput : public berry::IEditorInput {

public:
  berryObjectMacro(QmitkXnatObjectEditorInput);

  QmitkXnatObjectEditorInput(ctkXnatObject* object);
  ~QmitkXnatObjectEditorInput();

  /// \brief Returns the kept ctkXnatObject.
  ctkXnatObject* GetXnatObject() const;

  virtual bool Exists() const;
  virtual QString GetName() const;
  virtual QString GetToolTipText() const;
  virtual bool operator==(const berry::Object* o) const;

private:

  ctkXnatObject* m_Object;
};

#endif /*QMITKXNATOBJECTEDITORINPUT_H_*/
