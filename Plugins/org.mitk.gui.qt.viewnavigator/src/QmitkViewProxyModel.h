/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkViewProxyModel_h
#define QmitkViewProxyModel_h

#include <QSortFilterProxyModel>

class QmitkViewProxyModel : public QSortFilterProxyModel
{
public:
  explicit QmitkViewProxyModel(QObject* parent = nullptr);
  ~QmitkViewProxyModel() override;

private:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};

#endif
