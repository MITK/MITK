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

#include "berryIPerspectiveListener.h"

#include "berryIWorkbenchPage.h"

namespace berry {

void
IPerspectiveListener::Events
::AddListener(IPerspectiveListener* listener)
{
  if (listener == NULL) return;

  Types types = listener->GetPerspectiveEventTypes();

  if (types & ACTIVATED)
    this->perspectiveActivated += Delegate2(listener, &IPerspectiveListener::PerspectiveActivated);
  if (types & CHANGED)
    this->perspectiveChanged += PerspChangedDelegate(listener, &IPerspectiveListener::PerspectiveChanged);
  if (types & PART_CHANGED)
    this->perspectivePartChanged += PerspPartChangedDelegate(listener, &IPerspectiveListener::PerspectiveChanged);
  if (types & OPENED)
    this->perspectiveOpened += Delegate2(listener, &IPerspectiveListener::PerspectiveOpened);
  if (types & CLOSED)
    this->perspectiveClosed += Delegate2(listener, &IPerspectiveListener::PerspectiveClosed);
  if (types & DEACTIVATED)
    this->perspectiveDeactivated += Delegate2(listener, &IPerspectiveListener::PerspectiveDeactivated);
  if (types & SAVED_AS)
    this->perspectiveSavedAs += PerspSavedAsDelegate(listener, &IPerspectiveListener::PerspectiveSavedAs);
  if (types & PRE_DEACTIVATE)
    this->perspectivePreDeactivate += Delegate2(listener, &IPerspectiveListener::PerspectivePreDeactivate);
}

void
IPerspectiveListener::Events
::RemoveListener(IPerspectiveListener* listener)
{
  if (listener == NULL) return;

  this->perspectiveActivated -= Delegate2(listener, &IPerspectiveListener::PerspectiveActivated);
  this->perspectiveChanged -= PerspChangedDelegate(listener, &IPerspectiveListener::PerspectiveChanged);
  this->perspectivePartChanged -= PerspPartChangedDelegate(listener, &IPerspectiveListener::PerspectiveChanged);
  this->perspectiveOpened -= Delegate2(listener, &IPerspectiveListener::PerspectiveOpened);
  this->perspectiveClosed -= Delegate2(listener, &IPerspectiveListener::PerspectiveClosed);
  this->perspectiveDeactivated -= Delegate2(listener, &IPerspectiveListener::PerspectiveDeactivated);
  this->perspectiveSavedAs -= PerspSavedAsDelegate(listener, &IPerspectiveListener::PerspectiveSavedAs);
  this->perspectivePreDeactivate -= Delegate2(listener, &IPerspectiveListener::PerspectivePreDeactivate);
}

IPerspectiveListener::~IPerspectiveListener()
{
}

void IPerspectiveListener::PerspectiveActivated(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer&  /*perspective*/)
{}

void IPerspectiveListener::PerspectiveChanged(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer&  /*perspective*/, const QString&  /*changeId*/)
{}

void IPerspectiveListener::PerspectiveChanged(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer& /*perspective*/,
          const IWorkbenchPartReference::Pointer&  /*partRef*/, const QString&  /*changeId*/)
{}

void IPerspectiveListener::PerspectiveOpened(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer&  /*perspective*/)
{}

void IPerspectiveListener::PerspectiveClosed(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer&  /*perspective*/)
{}

void IPerspectiveListener::PerspectiveDeactivated(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer&  /*perspective*/)
{}

void IPerspectiveListener::PerspectiveSavedAs(const SmartPointer<IWorkbenchPage>& /*page*/,
          const IPerspectiveDescriptor::Pointer& /*oldPerspective*/,
          const IPerspectiveDescriptor::Pointer&  /*newPerspective*/)
{}

void IPerspectiveListener::PerspectivePreDeactivate(const SmartPointer<IWorkbenchPage>& /*page*/,
      const IPerspectiveDescriptor::Pointer&  /*perspective*/)
{}

}
