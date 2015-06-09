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
#include <mitkStringProperty.h>
#include <mitkNodePredicateFirstLevel.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include "QmitkDataStorageFilterProxyModel.h"
#include "QmitkDataStorageTreeModel.h"
#include "QmitkNodeDescriptorManager.h"
#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

#include <QIcon>
#include <QMimeData>
#include <QTextStream>

#include <map>

QmitkDataStorageFilterProxyModel::QmitkDataStorageFilterProxyModel(QObject* parent)
: QSortFilterProxyModel(parent)
{
}

QmitkDataStorageFilterProxyModel::~QmitkDataStorageFilterProxyModel()
{
}


void QmitkDataStorageFilterProxyModel::AddFilterPredicate(mitk::NodePredicateBase::Pointer pred)
{
    m_Predicates.insert(pred);
    this->invalidateFilter();
}

bool QmitkDataStorageFilterProxyModel::RemoveFilterPredicate(mitk::NodePredicateBase::Pointer pred)
{
    bool removed = m_Predicates.erase(pred) != 0;
    if (removed) {
        this->invalidateFilter();
    }
    return removed;
}

bool QmitkDataStorageFilterProxyModel::HasFilterPredicate(mitk::NodePredicateBase::Pointer pred)
{
    return m_Predicates.find(pred) != m_Predicates.end();
}

bool QmitkDataStorageFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QmitkDataStorageTreeModel* model = dynamic_cast<QmitkDataStorageTreeModel*>(this->sourceModel());
    assert(model);

    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    for (FilterPredicatesCollection::const_iterator iter = m_Predicates.begin(); iter != m_Predicates.end(); ++iter) {
        if ((*iter)->CheckNode(model->GetNode(index0))) {
            return false;
        }
    }

    return true;
}


