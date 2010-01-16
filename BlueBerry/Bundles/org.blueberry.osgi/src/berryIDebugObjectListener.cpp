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

#include "berryIDebugObjectListener.h"

namespace berry {

void IDebugObjectListener::Events::AddListener(IDebugObjectListener::Pointer l)
{
  if (!l) return;

  Types t = l->GetEventTypes();

  if (t & OBJECT_CREATED) objCreatedEvent += ObjDelegate(l.GetPointer(), &IDebugObjectListener::ObjectCreated);
  if (t & OBJECT_DESTROYED) objDestroyedEvent += ObjDelegate(l.GetPointer(), &IDebugObjectListener::ObjectDestroyed);
  if (t & OBJECT_TRACING) objTracingEvent += TraceDelegate(l.GetPointer(), &IDebugObjectListener::ObjectTracingChanged);
  if (t & SMARTPOINTER_CREATED) spCreatedEvent += SPDelegate(l.GetPointer(), &IDebugObjectListener::SmartPointerCreated);
  if (t & SMARTPOINTER_CREATED) spDestroyedEvent += SPDelegate(l.GetPointer(), &IDebugObjectListener::SmartPointerDestroyed);
}

void IDebugObjectListener::Events::RemoveListener(IDebugObjectListener::Pointer l)
{
  if (!l) return;

  objCreatedEvent -= ObjDelegate(l.GetPointer(), &IDebugObjectListener::ObjectCreated);
  objDestroyedEvent -= ObjDelegate(l.GetPointer(), &IDebugObjectListener::ObjectDestroyed);
  objTracingEvent -= TraceDelegate(l.GetPointer(), &IDebugObjectListener::ObjectTracingChanged);
  spCreatedEvent -= SPDelegate(l.GetPointer(), &IDebugObjectListener::SmartPointerCreated);
  spDestroyedEvent -= SPDelegate(l.GetPointer(), &IDebugObjectListener::SmartPointerDestroyed);
}

}
