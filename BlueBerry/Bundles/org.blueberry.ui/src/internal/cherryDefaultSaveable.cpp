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
  IWorkbenchPart::Pointer _part(part);
  if (_part.Cast<ISaveablePart> () != 0)
  {
    _part.Cast<ISaveablePart> ()->DoSave(/*monitor*/);
  }
}

std::string DefaultSaveable::GetName() const
{
  return part.Lock()->GetPartName();
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
  return part.Lock()->GetTitleToolTip();
}

bool DefaultSaveable::IsDirty() const
{
  IWorkbenchPart::Pointer _part(part);
  if (_part.Cast<ISaveablePart> () != 0)
  {
    return _part.Cast<ISaveablePart> ()->IsDirty();
  }
  return false;
}

bool DefaultSaveable::operator<(const Saveable* obj) const
{
  if (this == obj)
    return false;
  if (obj == 0)
    return true;

  const DefaultSaveable* other = dynamic_cast<const DefaultSaveable*> (obj);
  if (part.Expired())
  {
    return !other->part.Expired();
  }
  else
    return part < other->part;
}

bool DefaultSaveable::Show(IWorkbenchPage::Pointer page)
{
  IWorkbenchPart::Pointer _part(part);
  IWorkbenchPartReference::Pointer reference = page->GetReference(_part);
  if (reference != 0)
  {
    page->Activate(_part);
    return true;
  }
  if (_part.Cast<IViewPart> () != 0)
  {
    IViewPart::Pointer viewPart = _part.Cast<IViewPart> ();
    try
    {
      page->ShowView(viewPart->GetViewSite()->GetId(),
          viewPart ->GetViewSite()->GetSecondaryId(),
          IWorkbenchPage::VIEW_ACTIVATE);
    } catch (PartInitException& /*e*/)
    {
      return false;
    }
    return true;
  }
  return false;
}

}
