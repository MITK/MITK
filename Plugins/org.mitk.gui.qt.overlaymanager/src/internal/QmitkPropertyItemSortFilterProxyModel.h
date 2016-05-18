/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkPropertyItemSortFilterProxyModel_h
#define QmitkPropertyItemSortFilterProxyModel_h

#include <QSortFilterProxyModel>

class QmitkPropertyItemSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  explicit QmitkPropertyItemSortFilterProxyModel(QObject* parent = nullptr);
  ~QmitkPropertyItemSortFilterProxyModel();

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  bool FilterAcceptsAnyChildRow(const QModelIndex& sourceParent) const;
};

#endif
