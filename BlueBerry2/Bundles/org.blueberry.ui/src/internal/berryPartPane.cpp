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

#include "berryPartPane.h"

#include "berryWorkbenchPage.h"
#include "berryPartStack.h"
#include "berryEditorAreaHelper.h"
#include "berryPerspective.h"
#include "berryPartStack.h"
#include "berryDragUtil.h"

#include "../tweaklets/berryGuiWidgetsTweaklet.h"
#include "../tweaklets/berryWorkbenchPageTweaklet.h"

namespace berry
{

PartPane::Sashes::Sashes() :
  left(0), right(0), top(0), bottom(0)
{
}

PartPane::PartPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage* workbenchPage)
 : StackablePart(partReference->GetId()),
   control(0), inLayout(true), busy(false), hasFocus(false)
{
  //super(partReference.getId());
  this->partReference = partReference;
  this->page = workbenchPage;
}

void PartPane::CreateControl(void* parent) {
  if (this->GetControl() != 0)
  {
    return;
  }

  partReference.Lock()->AddPropertyListener(IPropertyChangeListener::Pointer(this));

  // Create view form.
  control = Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreatePaneControl(parent);

  // the part should never be visible by default.  It will be made visible
  // by activation.  This allows us to have views appear in tabs without
  // becoming active by default.
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(control, false);
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->MoveAbove(control, 0);

  // Create a title bar.
  //this->CreateTitleBar();


  // When the pane or any child gains focus, notify the workbench.
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(control, GuiTk::IControlListener::Pointer(this));

  //control.addTraverseListener(traverseListener);
}

bool PartPane::IsPlaceHolder()
{
  return false;
}

PartPane::~PartPane()
{
//  super.dispose();
//
  this->Register();

  if (control != 0)
  {
    BERRY_DEBUG << "Deleting PartPane control";
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->RemoveControlListener(control, GuiTk::IControlListener::Pointer(this));
//    control.removeTraverseListener(traverseListener);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->Dispose(control);
    control = 0;
  }
//  if ((paneMenuManager != null))
//  {
//    paneMenuManager.dispose();
//    paneMenuManager = null;
//  }
//
  if (!partReference.Expired())
  {
    partReference.Lock()->RemovePropertyListener(IPropertyChangeListener::Pointer(this));
  }
//  partReference.removePartPropertyListener(this);

  this->UnRegister(false);
}

void PartPane::DoHide()
{
  if (partReference.Lock().Cast<IViewReference>() != 0)
  {
    this->GetPage()->HideView(partReference.Lock().Cast<IViewReference>());
  }
  else if (partReference.Lock().Cast<IEditorReference>() != 0)
  {
    this->GetPage()->CloseEditor(partReference.Lock().Cast<IEditorReference>(), true);
  }
}

Rectangle PartPane::GetParentBounds()
{
  void* ctrl = this->GetControl();

  if (this->GetContainer() != 0 && this->GetContainer().Cast<LayoutPart>() != 0) {
      LayoutPart::Pointer part = this->GetContainer().Cast<LayoutPart>();

      if (part->GetControl() != 0) {
          ctrl = part->GetControl();
      }
  }

  return DragUtil::GetDisplayBounds(ctrl);
}

void* PartPane::GetControl()
{
  return control;
}

IWorkbenchPartReference::Pointer PartPane::GetPartReference() const
{
  return partReference.Lock();
}

void PartPane::ControlActivated(GuiTk::ControlEvent::Pointer /*e*/)
{
  if (inLayout)
  {
    this->RequestActivation();
  }
}

GuiTk::IControlListener::Events::Types PartPane::GetEventTypes() const
{
  return GuiTk::IControlListener::Events::ACTIVATED;
}

void PartPane::MoveAbove(void* refControl)
{
  if (this->GetControl() != 0)
  {
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->MoveAbove(this->GetControl(), refControl);
  }
}

void PartPane::RequestActivation()
{
  IWorkbenchPart::Pointer part = partReference.Lock()->GetPart(true);

  this->page->RequestActivation(part);
}

//PartStack::Pointer PartPane::GetStack()
//{
//  return partStack;
//}

PartPane::Sashes PartPane::FindSashes()
{
  Sashes result;

  IStackableContainer::Pointer container = this->GetContainer();

  if (container == 0) {
      return result;
  }

  container->FindSashes(result);
  return result;
}

WorkbenchPage::Pointer PartPane::GetPage()
{
  return WorkbenchPage::Pointer(page);
}

void PartPane::SetContainer(IStackableContainer::Pointer container)
{

  if (hasFocus)
  {
    IStackableContainer::Pointer oldContainer = this->GetContainer();
    if (PartStack::Pointer oldStack = oldContainer.Cast<PartStack>())
    {
      oldStack->SetActive(StackPresentation::AS_INACTIVE);
    }
    if (PartStack::Pointer newContainer = container.Cast<PartStack>())
    {
      newContainer->SetActive(StackPresentation::AS_ACTIVE_FOCUS);
    }
  }

  void* containerControl = container == 0 ? 0 : container.Cast<LayoutPart>()->GetControl();

  if (containerControl != 0)
  {
    void* control = this->GetControl();
    void* newParent = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(containerControl);
    if (control != 0 && newParent != Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(control))
    {
      this->Reparent(newParent);
    }
  }

  StackablePart::SetContainer(container);
}

void PartPane::Reparent(void* newParent)
{
  void* control = this->GetControl();

  GuiWidgetsTweaklet::Pointer guiTweaklet = Tweaklets::Get(GuiWidgetsTweaklet::KEY);
  if ((control == 0) || (guiTweaklet->GetParent(control) == newParent))
  {
    return;
  }

  if (guiTweaklet->IsReparentable(control))
  {
    // make control small in case it is not resized with other controls
    //control.setBounds(0, 0, 0, 0);
    // By setting the control to disabled before moving it,
    // we ensure that the focus goes away from the control and its children
    // and moves somewhere else
    bool enabled = guiTweaklet->GetEnabled(control);
    guiTweaklet->SetEnabled(control, false);
    guiTweaklet->SetParent(control, newParent);
    guiTweaklet->SetEnabled(control, enabled);
    guiTweaklet->MoveAbove(control, 0);
  }
}

void PartPane::ShowFocus(bool inFocus)
{
  if (partReference.Lock().Cast<IViewReference>() != 0)
  {
    hasFocus = inFocus;
  }

  if (PartStack::Pointer stack = this->GetContainer().Cast<PartStack>())
  {
    if (partReference.Lock().Cast<IViewReference>() != 0)
    {
      stack->SetActive(inFocus ? StackPresentation::AS_ACTIVE_FOCUS
                              : StackPresentation::AS_INACTIVE);
    }
    else if (partReference.Lock().Cast<IEditorReference>() != 0)
    {
      if (inFocus)
      {
        page->GetEditorPresentation()->SetActiveWorkbook(stack, true);
      }
      else
      {
        stack->SetActive(page->GetEditorPresentation()->GetActiveWorkbook() == stack ?
            StackPresentation::AS_ACTIVE_NOFOCUS : StackPresentation::AS_INACTIVE);
      }
    }
  }
}

PartStack::Pointer PartPane::GetStack()
{
  IStackableContainer::Pointer container = this->GetContainer();
  return container.Cast<PartStack>();
}

void PartPane::SetVisible(bool makeVisible)
{
  // Avoid redundant visibility changes
  if (makeVisible == this->GetVisible())
  {
    return;
  }

  if (makeVisible)
  {
    partReference.Lock()->GetPart(true);
  }

  if (this->GetControl() != 0)
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(this->GetControl(), makeVisible);

  partReference.Lock().Cast<WorkbenchPartReference>()->FireVisibilityChange();
}

bool PartPane::GetVisible()
{
  if (this->GetControl() != 0)
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetVisible(this->GetControl());
  return false;
}

void PartPane::SetFocus()
{
  this->RequestActivation();

  IWorkbenchPart::Pointer part = partReference.Lock()->GetPart(true);
  if (part.IsNotNull())
  {
//    Control control = getControl();
//    if (!SwtUtil.isFocusAncestor(control))
//    {
      // First try to call part.setFocus
      part->SetFocus();
    //}
  }
}

void PartPane::SetWorkbenchPage(WorkbenchPage::Pointer workbenchPage)
{
  this->page = workbenchPage.GetPointer();
}

void PartPane::DoDock()
{
  // do nothing
}

void PartPane::SetBusy(bool isBusy)
{
  if (isBusy != busy)
  {
    busy = isBusy;
    //firePropertyChange(IPresentablePart.PROP_BUSY);
  }
}

void PartPane::ShowHighlight()
{
  //No nothing by default
}

void* PartPane::GetToolBar()
{
  return 0;
}

bool PartPane::HasViewMenu()
{
  return false;
}

bool PartPane::IsBusy()
{
  return busy;
}

void PartPane::DescribeLayout(std::string& buf) const
{

  IWorkbenchPartReference::Pointer part = this->GetPartReference();

  if (part.IsNotNull())
  {
    buf.append(part->GetPartName());
    return;
  }
}

bool PartPane::IsCloseable()
{
  if (partReference.Lock().Cast<IViewReference>() != 0)
  {
    Perspective::Pointer perspective = page->GetActivePerspective();
    if (perspective == 0) {
        // Shouldn't happen -- can't have a ViewStack without a
        // perspective
        return true;
    }
    return perspective->IsCloseable(partReference.Lock().Cast<IViewReference>());
  }

  return true;
}

void PartPane::SetInLayout(bool inLayout)
{
  this->inLayout = inLayout;
}

bool PartPane::GetInLayout()
{
  return inLayout;
}

bool PartPane::AllowsAutoFocus()
{
  if (!inLayout)
  {
    return false;
  }

  //return super.allowsAutoFocus();
  return true;
}

void PartPane::RemoveContributions()
{

}

void PartPane::AddPropertyListener(IPropertyChangeListener::Pointer listener)
{
  propertyChangeEvents.AddListener(listener);
}

void PartPane::RemovePropertyListener(IPropertyChangeListener::Pointer listener)
{
  propertyChangeEvents.RemoveListener(listener);
}

void PartPane::FirePropertyChange(PropertyChangeEvent::Pointer event)
{
  propertyChangeEvents.propertyChange(event);
}

void PartPane::PropertyChange(PropertyChangeEvent::Pointer event)
{
  this->FirePropertyChange(event);
}

int PartPane::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{

  return partReference.Lock().Cast<WorkbenchPartReference>()->ComputePreferredSize(width,
      availableParallel, availablePerpendicular, preferredParallel);
}

int PartPane::GetSizeFlags(bool horizontal)
{
  return partReference.Lock().Cast<WorkbenchPartReference>()->GetSizeFlags(horizontal);
}

void PartPane::ShellActivated()
{

}

void PartPane::ShellDeactivated()
{

}

}
