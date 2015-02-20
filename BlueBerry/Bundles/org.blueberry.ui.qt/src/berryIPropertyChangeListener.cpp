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

#include "berryIPropertyChangeListener.h"

#include "berryIWorkbenchPartConstants.h"
#include <berryObjects.h>

namespace berry {

IPropertyChangeListener::~IPropertyChangeListener()
{
}

void
IPropertyChangeListener::Events
::AddListener(IPropertyChangeListener* listener)
{
  if (listener == NULL) return;

  this->propertyChange += Delegate(listener, &IPropertyChangeListener::PropertyChange);
}

void
IPropertyChangeListener::Events
::RemoveListener(IPropertyChangeListener* listener)
{
  if (listener == NULL) return;

  this->propertyChange -= Delegate(listener, &IPropertyChangeListener::PropertyChange);
}

void IPropertyChangeListener::PropertyChange(const PropertyChangeEvent::Pointer& event)
{
  if (event->GetProperty() == IWorkbenchPartConstants::INTEGER_PROPERTY)
  {
    this->PropertyChange(event->GetSource(), event->GetNewValue().Cast<ObjectInt>()->GetValue());
  }
}

}
