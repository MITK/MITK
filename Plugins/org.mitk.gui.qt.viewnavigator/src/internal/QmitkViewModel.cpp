/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewModel.h"

#include "QmitkCategoryItem.h"
#include "QmitkViewItem.h"

#include <berryIWorkbenchPartReference.h>
#include <berryPlatformUI.h>

namespace
{
  QString GetCategory(const berry::IViewDescriptor* view)
  {
    auto categoryPath = view->GetCategoryPath();

    return !categoryPath.isEmpty()
      ? categoryPath.first()
      : "";
  }
}

QmitkViewModel::QmitkViewModel(QObject* parent)
  : QStandardItemModel(parent)
{
  const auto viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
  const auto views = viewRegistry->GetViews();

  for (const auto& view : views)
  {
    // Ignore internal views and self (View Navigator).
    if (view->IsInternal() || view->GetId() == "org.mitk.views.viewnavigator")
      continue;

    auto category = GetCategory(view.GetPointer());

    // If a view is not categorized, put it into a virtual "Other" category.
    if (category.isEmpty())
      category = "Other";

    // Get corresponding category item or create it on the fly as needed.
    auto categoryItem = this->GetCategoryItem(category);

    if (categoryItem == nullptr)
      categoryItem = this->CreateCategoryItem(category);

    // Add the new view item as child node to the corresponding category item.
    categoryItem->appendRow(new QmitkViewItem(view.GetPointer()));
  }
}

QmitkViewModel::~QmitkViewModel()
{
}

QmitkCategoryItem* QmitkViewModel::CreateCategoryItem(const QString& category)
{
  auto categoryItem = new QmitkCategoryItem(category);
  this->appendRow(categoryItem);

  return categoryItem;
}

QmitkCategoryItem* QmitkViewModel::GetCategoryItem(const QString& category) const
{
  auto items = this->findItems(category);

  for (auto item : items)
  {
    auto categoryItem = dynamic_cast<QmitkCategoryItem*>(item);

    if (categoryItem != nullptr)
      return categoryItem;
  }

  return nullptr;
}

QmitkViewItem* QmitkViewModel::GetViewItemFromId(const QString& id) const
{
  const auto indices = this->match(this->index(0, 0), Qt::UserRole + 1, id, 1, Qt::MatchRecursive);

  return !indices.isEmpty()
    ? dynamic_cast<QmitkViewItem*>(this->itemFromIndex(indices.first()))
    : nullptr;
}
