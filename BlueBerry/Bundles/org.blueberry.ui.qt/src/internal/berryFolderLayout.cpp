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

#include "berryFolderLayout.h"
#include "berryPartPlaceholder.h"
#include "berryWorkbenchPlugin.h"
#include "berryPageLayout.h"
#include "berryLayoutHelper.h"

#include "berryUIException.h"

namespace berry
{
FolderLayout::FolderLayout(PageLayout::Pointer pageLayout, PartStack::Pointer folder,
    ViewFactory* viewFactory)
{
  this->folder = folder;
  this->viewFactory = viewFactory;
  this->pageLayout = pageLayout;
}

void FolderLayout::AddPlaceholder(const QString& viewId)
{
  if (!pageLayout->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  LayoutPart::Pointer newPart(new PartPlaceholder(viewId));
  this->LinkPartToPageLayout(viewId, newPart);

  // Add it to the folder layout.
  folder->Add(newPart);
}

void FolderLayout::AddView(const QString& viewId)
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
      throw PartInitException("View descriptor not found: " + viewId);
    }

      PartPane::Pointer newPart = LayoutHelper::CreateView(pageLayout->GetViewFactory(), viewId);
      this->LinkPartToPageLayout(viewId, newPart);
      folder->Add(newPart);
  }
  catch (const PartInitException& e)
  {
    // cannot safely open the dialog so log the problem
    WorkbenchPlugin::Log(this->GetClassName(), "AddView(const QString&)", e);
  }
}

QString FolderLayout::GetProperty(const QString& id)
{
  return folder->GetProperty(id);
}

void FolderLayout::SetProperty(const QString& id, const QString& value)
{
  folder->SetProperty(id, value);
}

void FolderLayout::LinkPartToPageLayout(const QString& viewId,
    LayoutPart::Pointer newPart)
{
  pageLayout->SetRefPart(viewId, newPart);
  pageLayout->SetFolderPart(viewId, folder);
  // force creation of the view layout rec
  pageLayout->GetViewLayoutRec(viewId, true);
}
}
