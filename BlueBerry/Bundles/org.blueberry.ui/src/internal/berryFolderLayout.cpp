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

#include "berryFolderLayout.h"
#include "berryPartPlaceholder.h"
#include "berryWorkbenchPlugin.h"
#include "berryPageLayout.h"
#include "berryLayoutHelper.h"

#include "../berryUIException.h"

namespace berry
{

FolderLayout::FolderLayout(PageLayout::Pointer pageLayout, PartStack::Pointer folder,
    ViewFactory* viewFactory)
{
  this->folder = folder;
  this->viewFactory = viewFactory;
  this->pageLayout = pageLayout;
}

void FolderLayout::AddPlaceholder(const std::string& viewId)
{
  if (!pageLayout->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  StackablePart::Pointer newPart(new PartPlaceholder(viewId));
  this->LinkPartToPageLayout(viewId, newPart);

  // Add it to the folder layout.
  folder->Add(newPart);
}

void FolderLayout::AddView(const std::string& viewId)
{
  if (pageLayout->CheckPartInLayout(viewId))
  {
    return;
  }

  try
  {
    IViewDescriptor::Pointer descriptor = viewFactory->GetViewRegistry()->Find(
        ViewFactory::ExtractPrimaryId(viewId));
    if (descriptor == 0)
    {
      throw PartInitException("View descriptor not found: " + viewId); //$NON-NLS-1$
    }

      PartPane::Pointer newPart = LayoutHelper::CreateView(pageLayout->GetViewFactory(), viewId);
      this->LinkPartToPageLayout(viewId, newPart);
      folder->Add(newPart);
  }
  catch (PartInitException& e)
  {
    // cannot safely open the dialog so log the problem
    WorkbenchPlugin::Log(this->GetClassName(), "AddView(const std::string&)", e); //$NON-NLS-1$
  }
}

std::string FolderLayout::GetProperty(const std::string& id)
{
  return folder->GetProperty(id);
}

void FolderLayout::SetProperty(const std::string& id, const std::string& value)
{
  folder->SetProperty(id, value);
}

void FolderLayout::LinkPartToPageLayout(const std::string& viewId,
    StackablePart::Pointer newPart)
{
  pageLayout->SetRefPart(viewId, newPart);
  pageLayout->SetFolderPart(viewId, folder);
  // force creation of the view layout rec
  pageLayout->GetViewLayoutRec(viewId, true);
}

}
