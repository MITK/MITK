/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCategoryItem.h"
#include "QmitkViewItem.h"

#include <berryIViewDescriptor.h>

QmitkCategoryItem::QmitkCategoryItem(const QString& category)
  : QStandardItem(category)
{
  auto font = this->font();
  font.setPointSizeF(font.pointSizeF() * 1.25);
  this->setFont(font);
}

QmitkCategoryItem::~QmitkCategoryItem()
{
}

bool QmitkCategoryItem::HasMatch(const QRegularExpression& re) const
{
  if (!this->hasChildren())
    return false;

  const int rowCount = this->rowCount();

  for (int row = 0; row < rowCount; ++row)
  {
    if (const auto* viewItem = dynamic_cast<QmitkViewItem*>(this->child(row)); viewItem != nullptr)
    {
      if (viewItem->Match(re))
        return true;
    }
  }

  return false;
}
