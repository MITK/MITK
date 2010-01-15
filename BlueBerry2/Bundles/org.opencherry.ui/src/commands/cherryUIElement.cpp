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

#include "cherryUIElement.h"

#include "../services/cherryIServiceLocator.h"
#include "../cherryImageDescriptor.h"

namespace cherry
{

UIElement::UIElement(SmartPointer<IServiceLocator> serviceLocator)
    throw(Poco::InvalidArgumentException) :
  serviceLocator(serviceLocator)
{
  if (!serviceLocator)
    throw Poco::InvalidArgumentException();
}

void UIElement::AddListener(SmartPointer<IUIElementListener> l)
{
  events.AddListener(l);
}

void UIElement::RemoveListener(SmartPointer<IUIElementListener> l)
{
  events.RemoveListener(l);
}

void UIElement::SetData(Object::Pointer data)
{
  this->data = data;
}

Object::Pointer UIElement::GetData() const
{
  return data;
}

SmartPointer<IServiceLocator> UIElement::GetServiceLocator() const
{
  return serviceLocator;
}

void UIElement::SetDropDownId(const std::string& id)
{
  // This does nothing.
}

}
