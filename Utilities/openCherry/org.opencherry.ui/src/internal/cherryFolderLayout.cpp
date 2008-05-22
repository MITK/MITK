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

#include "cherryFolderLayout.h"
#include "cherryPartPlaceholder.h"
#include "cherryWorkbenchPlugin.h"
#include "cherryPageLayout.h"

#include "../cherryUIException.h"

namespace cherry
{

FolderLayout::FolderLayout(PageLayout::Pointer pageLayout, PartStack::Pointer folder,
    ViewFactory* viewFactory)
{
  super();
  this.folder = folder;
  this.viewFactory = viewFactory;
  this.pageLayout = pageLayout;
}

void FolderLayout::AddPlaceholder(const std::string& viewId)
{
  if (!pageLayout->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  PartPlaceholder::Pointer newPart = new PartPlaceholder(viewId);
  this->LinkPartToPageLayout(viewId, newPart);

  // Add it to the folder layout.
  folder-Add(newPart);
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
    if (descriptor.IsNull())
    {
      throw PartInitException("View descriptor not found: " + viewId); //$NON-NLS-1$
    }
    
      ViewPane newPart = LayoutHelper.createView(pageLayout->GetViewFactory(), viewId);
      linkPartToPageLayout(viewId, newPart);
      folder.add(newPart);
  }
  catch (PartInitException& e)
  {
    // cannot safely open the dialog so log the problem
    WorkbenchPlugin::Log(this->GetNameOfClass(), "addView(String)", e); //$NON-NLS-1$
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
    LayoutPart::Pointer newPart)
{
  pageLayout->SetRefPart(viewId, newPart);
  pageLayout->SetFolderPart(viewId, folder);
  // force creation of the view layout rec
  pageLayout->GetViewLayoutRec(viewId, true);
}

}
