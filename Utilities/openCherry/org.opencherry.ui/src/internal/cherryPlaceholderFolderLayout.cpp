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

#include "cherryPlaceholderFolderLayout.h"

namespace cherry
{

PlaceholderFolderLayout::PlaceholderFolderLayout(
    PageLayout::Pointer pageLayout, ContainerPlaceholder::Pointer folder)
{
  super();
  this.placeholder = folder;
  this.pageLayout = pageLayout;
}

void PlaceholderFolderLayout::AddPlaceholder(const std::string& viewId)
{
  if (!pageLayout.checkValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  LayoutPart newPart = new PartPlaceholder(viewId);

  linkPartToPageLayout(viewId, newPart);

  // Add it to the placeholder layout.
  placeholder.add(newPart);
}

std::string PlaceholderFolderLayout::GetProperty(const std::string& id)
{
  LayoutPart folder = placeholder.getRealContainer();
  if (folder instanceof PartStack)
  {
    PartStack stack = (PartStack)folder;
    return stack.getProperty(id);
  }
  //throw not supported?
  return null;
}

void PlaceholderFolderLayout::SetProperty(const std::string& id,
    const std::string& value)
{
  LayoutPart folder = placeholder.getRealContainer();
  if (folder instanceof PartStack)
  {
    PartStack stack = (PartStack)folder;
    stack.setProperty(id, value);
  }
  //throw not supported?
}

void PlaceholderFolderLayout::LinkPartToPageLayout(const std::string& viewId,
    LayoutPart::Pointer newPart)
{
  pageLayout.setRefPart(viewId, newPart);
  // force creation of the view layout rec
  pageLayout.getViewLayoutRec(viewId, true);

  pageLayout.setFolderPart(viewId, placeholder);
  newPart.setContainer(placeholder);
}

}
