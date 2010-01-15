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

#include "cherryPresentablePart.h"

#include "../cherryIWorkbenchPartConstants.h"
#include "../cherryPartPane.h"
#include "cherryWorkbenchPage.h"

#include <osgi/framework/Objects.h>

namespace cherry
{

PresentablePart::
PropertyListenerProxy::PropertyListenerProxy(PresentablePart* p)
: part(p)
{
}

void
PresentablePart::
PropertyListenerProxy::PropertyChange(PropertyChangeEvent::Pointer e)
{
  part->FirePropertyChange(e);
}

IPropertyChangeListener::Pointer PresentablePart::GetPropertyListenerProxy()
{
  if (lazyPartPropertyChangeListener == 0)
  {
    lazyPartPropertyChangeListener = new PropertyListenerProxy(this);
  }
  return lazyPartPropertyChangeListener;
}

WorkbenchPartReference::Pointer PresentablePart::GetPartReference() const
{
  return part->GetPartReference().Cast<WorkbenchPartReference>();
}

void PresentablePart::FirePropertyChange(PropertyChangeEvent::Pointer event)
{
  partPropertyChangeEvents.propertyChange(event);
}

void PresentablePart::FirePropertyChange(int propId)
{
  ObjectInt::Pointer val(new ObjectInt(propId));
  Object::Pointer source(this);
  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(source, IWorkbenchPartConstants::INTEGER_PROPERTY, val, val));
  this->FirePropertyChange(event);
}

PresentablePart::PresentablePart(PartPane::Pointer part, void*  /*parent*/)
{
  enableInputs = true;
  enableOutputs = true;
  isVisible = false;
  isDirty = false;
  isBusy = false;
  hasViewMenu = false;

  this->part = part;
  this->GetPane()->AddPropertyListener(this->GetPropertyListenerProxy());
}

PartPane::Pointer PresentablePart::GetPane() const
{
  return part;
}

PresentablePart::~PresentablePart()
{
  // Ensure that the property listener is detached (necessary to prevent leaks)
  this->GetPane()->RemovePropertyListener(this->GetPropertyListenerProxy());
}

void PresentablePart::AddPropertyListener(IPropertyChangeListener::Pointer listener)
{
  partPropertyChangeEvents.AddListener(listener);
}

void PresentablePart::RemovePropertyListener(
    IPropertyChangeListener::Pointer listener)
{
  partPropertyChangeEvents.RemoveListener(listener);
}

void PresentablePart::SetBounds(const Rectangle& bounds)
{
  savedBounds = bounds;
  if (enableInputs && part != 0)
  {
    part->SetBounds(bounds);
  }
}

void PresentablePart::SetVisible(bool isVisible)
{
  this->isVisible = isVisible;
  if (enableInputs)
  {
    part->SetVisible(isVisible);
  }
}

void PresentablePart::SetFocus()
{
  if (part != 0)
  {
    if (part->GetPage()->GetActivePart()
        == part->GetPartReference()->GetPart(false))
    {
      part->SetFocus();
    }
    else
    {
      part->RequestActivation();
    }
  }
}

std::string PresentablePart::GetName() const
{
  if (enableOutputs)
  {
    return this->GetPartReference()->GetPartName();
  }
  return name;
}

std::string PresentablePart::GetTitle() const
{
  return this->GetPartReference()->GetPartName();
}

std::string PresentablePart::GetTitleStatus() const
{
  if (enableOutputs)
  {
    return this->GetPartReference()->GetContentDescription();
  }

  return titleStatus;
}

void* PresentablePart::GetTitleImage()
{

  if (enableOutputs)
  {
    return this->GetPartReference()->GetTitleImage();
  }

//  return PlatformUI.getWorkbench().getSharedImages().getImage(
//      ISharedImages.IMG_DEF_VIEW);
  return 0;
}

std::string PresentablePart::GetTitleToolTip() const
{
  return this->GetPartReference()->GetTitleToolTip();
}

bool PresentablePart::IsDirty() const
{
  if (enableOutputs)
  {
    return this->GetPartReference()->IsDirty();
  }
  return isDirty;
}

bool PresentablePart::IsBusy() const
{
  if (enableOutputs)
  {
    return part->IsBusy();
  }
  return isBusy;
}

void* PresentablePart::GetToolBar()
{
  if (enableOutputs)
  {
    return this->GetPane()->GetToolBar();
  }
  return 0;
}

bool PresentablePart::IsCloseable() const
{
  return part->IsCloseable();
}

void* PresentablePart::GetControl()
{
  return part->GetControl();
}

void PresentablePart::EnableOutputs(bool isActive)
{
  if (isActive == this->enableOutputs)
  {
    return;
  }

  this->enableOutputs = isActive;

  if (isActive)
  {
    if (isBusy != this->GetPane()->IsBusy())
    {
      this->FirePropertyChange(PROP_BUSY);
    }
    if (isDirty != this->IsDirty())
    {
      this->FirePropertyChange(PROP_DIRTY);
    }
    if (name != this->GetName())
    {
      this->FirePropertyChange(PROP_PART_NAME);
    }
    if (titleStatus != this->GetTitleStatus())
    {
      this->FirePropertyChange(PROP_CONTENT_DESCRIPTION);
    }
    if (hasViewMenu != this->GetPane()->HasViewMenu())
    {
      this->FirePropertyChange(PROP_PANE_MENU);
    }
    // Always assume that the toolbar and title has changed (keeping track of this for real
    // would be too expensive)
    this->FirePropertyChange(PROP_TOOLBAR);
    this->FirePropertyChange(PROP_TITLE);

    this->GetPane()->AddPropertyListener(this->GetPropertyListenerProxy());
  }
  else
  {
    this->GetPane()->RemovePropertyListener(this->GetPropertyListenerProxy());

    WorkbenchPartReference::Pointer ref = this->GetPartReference();
    isBusy = this->GetPane()->IsBusy();
    isDirty = ref->IsDirty();
    name = ref->GetPartName();
    titleStatus = ref->GetContentDescription();
    hasViewMenu = this->GetPane()->HasViewMenu();
    this->FirePropertyChange(PROP_TITLE);
    this->FirePropertyChange(PROP_TOOLBAR);
  }
}

void PresentablePart::EnableInputs(bool isActive)
{
  if (isActive == this->enableInputs)
  {
    return;
  }

  this->enableInputs = isActive;

  if (isActive)
  {
    if (isActive && part != 0)
    {
      part->SetBounds(savedBounds);
    }

    part->SetVisible(isVisible);
  }
}

std::string PresentablePart::GetPartProperty(const std::string& key) const
{
  return this->GetPartReference()->GetPartProperty(key);
}

int PresentablePart::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredResult)
{

  return this->GetPane()->ComputePreferredSize(width, availableParallel,
      availablePerpendicular, preferredResult);
}

int PresentablePart::GetSizeFlags(bool width)
{
  return this->GetPane()->GetSizeFlags(width);
}

}
