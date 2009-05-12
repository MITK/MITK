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

#include "cherryDefaultSaveable.h"

#include "../cherryIWorkbenchPart.h"
#include "../cherryIWorkbenchPage.h"
#include "../cherryUIException.h"
#include "../cherryImageDescriptor.h"

namespace cherry
{

DefaultSaveable::DefaultSaveable(IWorkbenchPart::Pointer _part) :
 part(_part)
{

}

void DefaultSaveable::DoSave(/*IProgressMonitor monitor*/)
{
  if (part.Cast<ISaveablePart> () != 0)
  {
    part.Cast<ISaveablePart>()->DoSave(/*monitor*/);
  }
}

std::string DefaultSaveable::GetName() const
{
  return part->GetPartName();
}

ImageDescriptor::Pointer DefaultSaveable::GetImageDescriptor() const
{
  //TODO DefaultSaveable GetImageDescriptor
//  Image image = part.getTitleImage();
//  if (image == null)
//  {
//    return null;
//  }
//  return ImageDescriptor.createFromImage(image);
  return ImageDescriptor::Pointer(0);
}

std::string DefaultSaveable::GetToolTipText() const
{
  return part->GetTitleToolTip();
}

bool DefaultSaveable::IsDirty() const
{
  if (part.Cast<ISaveablePart> () != 0)
  {
    return part.Cast<ISaveablePart>()->IsDirty();
  }
  return false;
}

bool DefaultSaveable::operator<(const Saveable* obj) const
{
  if (this == obj)
  return true;
  if (obj == 0)
  return false;

  const DefaultSaveable* other = dynamic_cast<const DefaultSaveable*>(obj);
  if (part == 0)
  {
    if (other->part != 0)
    return false;
  }
  else if (part != other->part)
  return false;
  return true;
}

bool DefaultSaveable::Show(IWorkbenchPage::Pointer page)
{
  IWorkbenchPartReference::Pointer reference = page->GetReference(part);
  if (reference != 0)
  {
    page->Activate(part);
    return true;
  }
  if (part.Cast<IViewPart>() != 0)
  {
    IViewPart::Pointer viewPart = part.Cast<IViewPart>();
    try
    {
      page->ShowView(viewPart->GetViewSite()->GetId(), viewPart
          ->GetViewSite()->GetSecondaryId(),
          IWorkbenchPage::VIEW_ACTIVATE);
    }
    catch (PartInitException& e)
    {
      return false;
    }
    return true;
  }
  return false;
}

}
