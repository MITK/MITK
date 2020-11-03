/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateFirstLevel.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkStringProperty.h>

#include "QmitkDataStorageFilterProxyModel.h"
#include "QmitkDataStorageTreeModel.h"
#include "QmitkNodeDescriptorManager.h"
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>

#include <QIcon>
#include <QMimeData>
#include <QTextStream>

#include <map>

QmitkDataStorageFilterProxyModel::QmitkDataStorageFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
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
  if (removed)
  {
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
  QmitkDataStorageTreeModel *model = dynamic_cast<QmitkDataStorageTreeModel *>(this->sourceModel());
  assert(model);

  QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

  for (FilterPredicatesCollection::const_iterator iter = m_Predicates.begin(); iter != m_Predicates.end(); ++iter)
  {
    if ((*iter)->CheckNode(model->GetNode(index0)))
    {
      return false;
    }
  }

  return true;
}
