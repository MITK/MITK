/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkViewModel_h
#define QmitkViewModel_h

#include <QStandardItemModel>

class QmitkCategoryItem;
class QmitkViewItem;

class QmitkViewModel : public QStandardItemModel
{
public:
  explicit QmitkViewModel(QObject* parent = nullptr);
  ~QmitkViewModel() override;

  QmitkCategoryItem* GetCategoryItem(const QString& category) const;
  QmitkViewItem* GetViewItemFromId(const QString& id) const;

private:
  QmitkCategoryItem* CreateCategoryItem(const QString& category);
};

#endif
