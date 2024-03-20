/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCategoryItem_h
#define QmitkCategoryItem_h

#include <QRegularExpression>
#include <QStandardItem>

class QmitkCategoryItem : public QStandardItem
{
public:
  explicit QmitkCategoryItem(const QString& category);
  ~QmitkCategoryItem() override;

  bool HasMatch(const QRegularExpression& re) const;
};

#endif
