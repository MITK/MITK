/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageFilterProxyModel_h
#define QmitkDataStorageFilterProxyModel_h

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QSortFilterProxyModel>

#include <set>

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkDataStorageFilterProxyModel : public QSortFilterProxyModel
{
  //# CTORS,DTOR
public:
  QmitkDataStorageFilterProxyModel(QObject *parent = nullptr);
  ~QmitkDataStorageFilterProxyModel() override;

public:
  ///
  /// If the predicate pred returns true, the node will be hidden in the data manager view
  ///
  void AddFilterPredicate(mitk::NodePredicateBase::Pointer pred);

  ///
  /// Remove a predicate from the list of filters. Returns true if pred was found and removed.
  ///
  bool RemoveFilterPredicate(mitk::NodePredicateBase::Pointer pred);

  ///
  /// Check if predicate is present in the list of filtering predicates.
  ///
  bool HasFilterPredicate(mitk::NodePredicateBase::Pointer pred);

  //#
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

protected:
  typedef std::set<mitk::NodePredicateBase::Pointer> FilterPredicatesCollection;
  FilterPredicatesCollection m_Predicates;
};

#endif
