/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryElementReference.h"

#include <berryUIElement.h>

namespace berry {

ElementReference::ElementReference(const QString& id, const SmartPointer<UIElement>& adapt,
                 const QHash<QString, Object::Pointer>& parms)
  : commandId(id), element(adapt), parameters(parms)
{
}

SmartPointer<UIElement> ElementReference::GetElement() const
{
  return element;
}

QString ElementReference::GetCommandId() const
{
  return commandId;
}

QHash<QString, Object::Pointer> ElementReference::GetParameters() const
{
  return parameters;
}

void ElementReference::AddParameter(const QString &name, const Object::Pointer &value)
{
  parameters.insert(name, value);
}

}
