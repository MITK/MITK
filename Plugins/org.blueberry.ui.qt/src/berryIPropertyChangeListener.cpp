/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  if (listener == nullptr) return;

  this->propertyChange += Delegate(listener, &IPropertyChangeListener::PropertyChange);
}

void
IPropertyChangeListener::Events
::RemoveListener(IPropertyChangeListener* listener)
{
  if (listener == nullptr) return;

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
