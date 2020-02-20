/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIDebugObjectListener.h"

namespace berry {

void IDebugObjectListener::Events::AddListener(IDebugObjectListener* l)
{
  if (l == nullptr) return;

  Types t = l->GetEventTypes();

  if (t & OBJECT_CREATED) objCreatedEvent += ObjDelegate(l, &IDebugObjectListener::ObjectCreated);
  if (t & OBJECT_DESTROYED) objDestroyedEvent += ObjDelegate(l, &IDebugObjectListener::ObjectDestroyed);
  if (t & OBJECT_TRACING) objTracingEvent += TraceDelegate(l, &IDebugObjectListener::ObjectTracingChanged);
  if (t & SMARTPOINTER_CREATED) spCreatedEvent += SPDelegate(l, &IDebugObjectListener::SmartPointerCreated);
  if (t & SMARTPOINTER_CREATED) spDestroyedEvent += SPDelegate(l, &IDebugObjectListener::SmartPointerDestroyed);
}

void IDebugObjectListener::Events::RemoveListener(IDebugObjectListener* l)
{
  if (l == nullptr) return;

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
