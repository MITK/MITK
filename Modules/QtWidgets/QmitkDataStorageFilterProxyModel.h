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


#ifndef QMITKDATASTORAGEFILTERPROXYMODEL_H_
#define QMITKDATASTORAGEFILTERPROXYMODEL_H_

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QSortFilterProxyModel>

#include <set>

/// \ingroup QmitkModule
class QMITK_EXPORT QmitkDataStorageFilterProxyModel : public QSortFilterProxyModel
{
//# CTORS,DTOR
public:
    QmitkDataStorageFilterProxyModel(QObject* parent = 0);
    ~QmitkDataStorageFilterProxyModel();

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

#endif /* QMITKDATASTORAGEFILTERPROXYMODEL_H_ */
