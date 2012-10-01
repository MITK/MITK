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
    PageLayout* pageLayout, ContainerPlaceholder::Pointer folder)
{
  this->placeholder = folder;
  this->pageLayout = pageLayout;
}

void PlaceholderFolderLayout::AddPlaceholder(const QString& viewId)
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

QString PlaceholderFolderLayout::GetProperty(const QString& id)
{
  LayoutPart::Pointer folder = placeholder->GetRealContainer();
  if (folder.Cast<PartStack>() != 0)
  {
    return folder.Cast<PartStack>()->GetProperty(id);
  }
  //throw not supported?
  return "";
}

void PlaceholderFolderLayout::SetProperty(const QString& id,
    const QString& value)
{
  LayoutPart::Pointer folder = placeholder->GetRealContainer();
  if (folder.Cast<PartStack>() != 0)
  {
    folder.Cast<PartStack>()->SetProperty(id, value);
  }
  //throw not supported?
}

void PlaceholderFolderLayout::LinkPartToPageLayout(const QString& viewId,
    LayoutPart::Pointer newPart)
{
  pageLayout->SetRefPart(viewId, newPart);
  // force creation of the view layout rec
  pageLayout->GetViewLayoutRec(viewId, true);

  pageLayout->SetFolderPart(viewId, placeholder);
  newPart->SetContainer(placeholder);
}

}
