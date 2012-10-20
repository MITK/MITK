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
