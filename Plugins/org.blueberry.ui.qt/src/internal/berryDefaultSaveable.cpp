/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryDefaultSaveable.h"

#include "berryIWorkbenchPart.h"
#include "berryIWorkbenchPage.h"
#include "berryUIException.h"

#include <QIcon>

namespace berry
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

QString DefaultSaveable::GetName() const
{
  return part.Lock()->GetPartName();
}

QIcon DefaultSaveable::GetImageDescriptor() const
{
  return part.Lock()->GetTitleImage();
}

QString DefaultSaveable::GetToolTipText() const
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

uint DefaultSaveable::HashCode() const
{
  return part.Lock()->HashCode();
}

bool DefaultSaveable::operator<(const Object* obj) const
{
  if (this == obj)
    return false;
  if (obj == nullptr)
    return true;

  const DefaultSaveable* other = dynamic_cast<const DefaultSaveable*> (obj);
  if (other == nullptr) return true;

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
                     viewPart->GetViewSite()->GetSecondaryId(),
                     IWorkbenchPage::VIEW_ACTIVATE);
    } catch (const PartInitException& /*e*/)
    {
      return false;
    }
    return true;
  }
  return false;
}

}
