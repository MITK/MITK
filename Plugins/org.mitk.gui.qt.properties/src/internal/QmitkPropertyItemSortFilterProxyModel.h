/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyItemSortFilterProxyModel_h
#define QmitkPropertyItemSortFilterProxyModel_h

#include <QSortFilterProxyModel>

class QmitkPropertyItemSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  explicit QmitkPropertyItemSortFilterProxyModel(QObject* parent = nullptr);
  ~QmitkPropertyItemSortFilterProxyModel() override;

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  bool FilterAcceptsAnyChildRow(const QModelIndex& sourceParent) const;
};

#endif
