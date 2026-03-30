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

/**
 * \ingroup QmitkModule
 * \brief A QSortFilterProxyModel that hides data nodes matching one or more predicates.
 *
 * This proxy model filters rows from a source model (typically a DataStorage-backed
 * model) by evaluating a set of mitk::NodePredicateBase instances. If any predicate
 * returns true for a given data node, the corresponding row is hidden.
 *
 * \sa QmitkAbstractDataStorageModel
 * \sa QmitkDataStorageTreeModel
 */
class MITKQTWIDGETS_EXPORT QmitkDataStorageFilterProxyModel : public QSortFilterProxyModel
{
public:
  /**
   * \brief Constructs the filter proxy model.
   * \param[in] parent Optional parent QObject.
   */
  QmitkDataStorageFilterProxyModel(QObject *parent = nullptr);
  ~QmitkDataStorageFilterProxyModel() override;

public:
  /**
   * \brief Adds a filter predicate. Nodes matching this predicate will be hidden.
   * \param[in] pred The predicate to add.
   */
  void AddFilterPredicate(mitk::NodePredicateBase::Pointer pred);

  /**
   * \brief Removes a filter predicate from the filter list.
   * \param[in] pred The predicate to remove.
   * \return True if the predicate was found and removed, false otherwise.
   */
  bool RemoveFilterPredicate(mitk::NodePredicateBase::Pointer pred);

  /**
   * \brief Checks whether a predicate is currently in the filter list.
   * \param[in] pred The predicate to check for.
   * \return True if the predicate is present, false otherwise.
   */
  bool HasFilterPredicate(mitk::NodePredicateBase::Pointer pred);

  //#
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

protected:
  typedef std::set<mitk::NodePredicateBase::Pointer> FilterPredicatesCollection;
  FilterPredicatesCollection m_Predicates;
};

#endif
