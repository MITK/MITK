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

#include "berryUIElement.h"

#include "../services/berryIServiceLocator.h"

namespace berry
{

UIElement::UIElement(IServiceLocator* serviceLocator)
  : serviceLocator(serviceLocator)
{
  if (!serviceLocator)
    throw std::invalid_argument("serviceLocator must not be NULL");
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
