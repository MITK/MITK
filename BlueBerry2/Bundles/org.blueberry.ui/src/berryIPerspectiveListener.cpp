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

#include "berryIPerspectiveListener.h"

#include "berryIWorkbenchPage.h"

namespace berry {

void
IPerspectiveListener::Events
::AddListener(IPerspectiveListener::Pointer listener)
{
  if (listener.IsNull()) return;

  Types types = listener->GetPerspectiveEventTypes();

  if (types & ACTIVATED)
    this->perspectiveActivated += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveActivated);
  if (types & CHANGED)
    this->perspectiveChanged += PerspChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  if (types & PART_CHANGED)
    this->perspectivePartChanged += PerspPartChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  if (types & OPENED)
    this->perspectiveOpened += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveOpened);
  if (types & CLOSED)
    this->perspectiveClosed += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveClosed);
  if (types & DEACTIVATED)
    this->perspectiveDeactivated += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveDeactivated);
  if (types & SAVED_AS)
    this->perspectiveSavedAs += PerspSavedAsDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveSavedAs);
  if (types & PRE_DEACTIVATE)
    this->perspectivePreDeactivate += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectivePreDeactivate);
}

void
IPerspectiveListener::Events
::RemoveListener(IPerspectiveListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->perspectiveActivated -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveActivated);
  this->perspectiveChanged -= PerspChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  this->perspectivePartChanged -= PerspPartChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  this->perspectiveOpened -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveOpened);
  this->perspectiveClosed -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveClosed);
  this->perspectiveDeactivated -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveDeactivated);
  this->perspectiveSavedAs -= PerspSavedAsDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveSavedAs);
  this->perspectivePreDeactivate -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectivePreDeactivate);
}

void IPerspectiveListener::PerspectiveActivated(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer  /*perspective*/)
{}

void IPerspectiveListener::PerspectiveChanged(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer  /*perspective*/, const std::string&  /*changeId*/) 
{}

void IPerspectiveListener::PerspectiveChanged(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer perspective,
          IWorkbenchPartReference::Pointer  /*partRef*/, const std::string&  /*changeId*/) 
{}

void IPerspectiveListener::PerspectiveOpened(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer  /*perspective*/) 
{}

void IPerspectiveListener::PerspectiveClosed(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer  /*perspective*/) 
{}

void IPerspectiveListener::PerspectiveDeactivated(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer  /*perspective*/) 
{}

void IPerspectiveListener::PerspectiveSavedAs(SmartPointer<IWorkbenchPage> /*page*/,
          IPerspectiveDescriptor::Pointer /*oldPerspective*/,
          IPerspectiveDescriptor::Pointer  /*newPerspective*/) 
{};

void IPerspectiveListener::PerspectivePreDeactivate(SmartPointer<IWorkbenchPage> /*page*/,
      IPerspectiveDescriptor::Pointer  /*perspective*/) 
{}

}
