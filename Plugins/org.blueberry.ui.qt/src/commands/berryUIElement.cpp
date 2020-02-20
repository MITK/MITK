/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryUIElement.h"

#include "../services/berryIServiceLocator.h"

namespace berry
{

UIElement::UIElement(IServiceLocator* serviceLocator)
  : serviceLocator(serviceLocator)
{
  if (!serviceLocator)
    throw std::invalid_argument("serviceLocator must not be nullptr");
}

IServiceLocator* UIElement::GetServiceLocator() const
{
  return serviceLocator;
}

void UIElement::SetDropDownId(const QString &/*id*/)
{
  // This does nothing.
}

}
