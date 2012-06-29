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

#include "berryPlaceholderFolderLayout.h"

namespace berry
{

PlaceholderFolderLayout::PlaceholderFolderLayout(
    PageLayout::Pointer pageLayout, ContainerPlaceholder::Pointer folder)
{
  this->placeholder = folder;
  this->pageLayout = pageLayout;
}

void PlaceholderFolderLayout::AddPlaceholder(const std::string& viewId)
{
  if (!pageLayout->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  LayoutPart::Pointer newPart(new PartPlaceholder(viewId));

  this->LinkPartToPageLayout(viewId, newPart);

  // Add it to the placeholder layout.
  placeholder->Add(newPart);
}

std::string PlaceholderFolderLayout::GetProperty(const std::string& id)
{
  LayoutPart::Pointer folder = placeholder->GetRealContainer();
  if (folder.Cast<PartStack>() != 0)
  {
    return folder.Cast<PartStack>()->GetProperty(id);
  }
  //throw not supported?
  return "";
}

void PlaceholderFolderLayout::SetProperty(const std::string& id,
    const std::string& value)
{
  LayoutPart::Pointer folder = placeholder->GetRealContainer();
  if (folder.Cast<PartStack>() != 0)
  {
    folder.Cast<PartStack>()->SetProperty(id, value);
  }
  //throw not supported?
}

void PlaceholderFolderLayout::LinkPartToPageLayout(const std::string& viewId,
    LayoutPart::Pointer newPart)
{
  pageLayout->SetRefPart(viewId, newPart);
  // force creation of the view layout rec
  pageLayout->GetViewLayoutRec(viewId, true);

  pageLayout->SetFolderPart(viewId, placeholder);
  newPart->SetContainer(placeholder);
}

}
