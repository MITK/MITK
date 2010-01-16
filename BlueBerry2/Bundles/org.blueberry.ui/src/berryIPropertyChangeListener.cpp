/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryIPropertyChangeListener.h"

#include "berryIWorkbenchPartConstants.h"
#include <osgi/framework/Objects.h>

namespace berry {

void
IPropertyChangeListener::Events
::AddListener(IPropertyChangeListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->propertyChange += Delegate(listener.GetPointer(), &IPropertyChangeListener::PropertyChange);
}

void
IPropertyChangeListener::Events
::RemoveListener(IPropertyChangeListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->propertyChange -= Delegate(listener.GetPointer(), &IPropertyChangeListener::PropertyChange);
}

void IPropertyChangeListener::PropertyChange(PropertyChangeEvent::Pointer event)
{
  if (event->GetProperty() == IWorkbenchPartConstants::INTEGER_PROPERTY)
  {
    this->PropertyChange(event->GetSource(), event->GetNewValue().Cast<ObjectInt>()->GetValue());
  }
}

}
