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

#include "berryIDebugObjectListener.h"

namespace berry {

void IDebugObjectListener::Events::AddListener(IDebugObjectListener* l)
{
  if (l == 0) return;

  Types t = l->GetEventTypes();

  if (t & OBJECT_CREATED) objCreatedEvent += ObjDelegate(l, &IDebugObjectListener::ObjectCreated);
  if (t & OBJECT_DESTROYED) objDestroyedEvent += ObjDelegate(l, &IDebugObjectListener::ObjectDestroyed);
  if (t & OBJECT_TRACING) objTracingEvent += TraceDelegate(l, &IDebugObjectListener::ObjectTracingChanged);
  if (t & SMARTPOINTER_CREATED) spCreatedEvent += SPDelegate(l, &IDebugObjectListener::SmartPointerCreated);
  if (t & SMARTPOINTER_CREATED) spDestroyedEvent += SPDelegate(l, &IDebugObjectListener::SmartPointerDestroyed);
}

void IDebugObjectListener::Events::RemoveListener(IDebugObjectListener* l)
{
  if (l == 0) return;

  objCreatedEvent -= ObjDelegate(l, &IDebugObjectListener::ObjectCreated);
  objDestroyedEvent -= ObjDelegate(l, &IDebugObjectListener::ObjectDestroyed);
  objTracingEvent -= TraceDelegate(l, &IDebugObjectListener::ObjectTracingChanged);
  spCreatedEvent -= SPDelegate(l, &IDebugObjectListener::SmartPointerCreated);
  spDestroyedEvent -= SPDelegate(l, &IDebugObjectListener::SmartPointerDestroyed);
}

IDebugObjectListener::~IDebugObjectListener()
{
}

}
