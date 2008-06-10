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

#include "cherryPartPane.h"

namespace cherry
{

PartPane::PartPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
 : control(0), inLayout(true)
{
  //super(partReference.getId());
  this->partReference = partReference;
  this->page = workbenchPage;
}

//void PartPane::CreateControl(void* parent)
//{
//  if (this->GetControl() != 0)
//  {
//    return;
//  }
//
//  //partReference.addPropertyListener(this);
//  //partReference.addPartPropertyListener(this);
//  
//  // Create view form.  
//  control = new Composite(parent, SWT.NONE);
//  control.setLayout(new FillLayout());
//  // the part should never be visible by default.  It will be made visible 
//  // by activation.  This allows us to have views appear in tabs without 
//  // becoming active by default.
//  control.setVisible(false);
//  control.moveAbove(null);
//  // Create a title bar.
//  createTitleBar();
//
//  // When the pane or any child gains focus, notify the workbench.
//  control.addListener(SWT.Activate, this);
//
//  control.addTraverseListener(traverseListener);
//}

PartPane::~PartPane()
{
//  super.dispose();
//
//  if ((control != null) && (!control.isDisposed()))
//  {
//    control.removeListener(SWT.Activate, this);
//    control.removeTraverseListener(traverseListener);
//    control.dispose();
//    control = null;
//  }
//  if ((paneMenuManager != null))
//  {
//    paneMenuManager.dispose();
//    paneMenuManager = null;
//  }
//
//  partReference.removePropertyListener(this);
//  partReference.removePartPropertyListener(this);
}

//Rectangle PartPane::GetBounds()
//{
//  return getControl().getBounds();
//}

void* PartPane::GetControl()
{
  return control;
}

IWorkbenchPartReference::Pointer PartPane::GetPartReference()
{
  return partReference;
}

//void PartPane::MoveAbove(void* refControl)
//{
//  if (getControl() != null)
//  {
//    getControl().moveAbove(refControl);
//  }
//}

void PartPane::RequestActivation()
{
  IWorkbenchPart::Pointer part = partReference->GetPart(true);

  this->page->RequestActivation(part);
}

void PartPane::SetContainer(ILayoutContainer::Pointer container)
{

//  if (container.Cast<LayoutPart>().IsNotNull())
//  {
//    LayoutPart part = (LayoutPart) container;
//
//    Control containerControl = part.getControl();
//
//    if (containerControl != null)
//    {
//      Control control = getControl();
//      Composite newParent = containerControl.getParent();
//      if (control != null && newParent != control.getParent())
//      {
//        reparent(newParent);
//      }
//    }
//  }
//  super.setContainer(container);
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
    partReference->GetPart(true);
  }

  this->SetControlVisible(makeVisible);

  //((WorkbenchPartReference) partReference).fireVisibilityChange();
}

bool PartPane::GetVisible()
{
  return this->GetControlVisible();
}

void PartPane::SetFocus()
{
  this->RequestActivation();

  IWorkbenchPart::Pointer part = partReference->GetPart(true);
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
  this->page = workbenchPage;
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

void PartPane::DescribeLayout(std::string& buf)
{

  IWorkbenchPartReference::Pointer part = this->GetPartReference();

  if (part.IsNotNull())
  {
    buf.append(part->GetPartName());
    return;
  }
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

int PartPane::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{

  return partReference.Cast<WorkbenchPartReference>()->ComputePreferredSize(width,
      availableParallel, availablePerpendicular, preferredParallel);
}

int PartPane::GetSizeFlags(bool horizontal)
{
  return partReference.Cast<WorkbenchPartReference>()->GetSizeFlags(horizontal);
}

}
