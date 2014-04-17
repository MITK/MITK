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

QmitkDataStorageFilterProxyModel::QmitkDataStorageFilterProxyModel( bool _ShowHelperObjects
                                                      , bool _ShowNodesContainingNoData
                                                      , QObject* parent )
: QSortFilterProxyModel(parent)
, m_ShowHelperObjects(_ShowHelperObjects)
, m_ShowNodesContainingNoData(_ShowNodesContainingNoData)
{
}

QmitkDataStorageFilterProxyModel::~QmitkDataStorageFilterProxyModel()
{
  //Removing all observers
  for ( NodeTagMapType::iterator dataIter = m_HelperObjectObserverTags.begin(); dataIter != m_HelperObjectObserverTags.end(); ++dataIter )
  {
      (*dataIter).first->GetProperty("helper object")->RemoveObserver( (*dataIter).second );
  }
  m_HelperObjectObserverTags.clear();
}


// void QmitkDataStorageFilterProxyModel::AddNode( const mitk::DataNode* node )
// {
//     if(node == 0
//       || m_DataStorage.IsNull()
//       || !m_DataStorage->Exists(node)
//       || m_Root->Find(node) != 0)
//       return;
// 
//     bool isHelperObject (false);
//     bool isHidden(false);
//     NodeTagMapType::iterator searchIter = m_HelperObjectObserverTags.find( const_cast<mitk::DataNode*>(node) );
//     if (node->GetBoolProperty("helper object", isHelperObject) && searchIter == m_HelperObjectObserverTags.end()) {
//         itk::SimpleMemberCommand<QmitkDataStorageFilterProxyModel>::Pointer command = itk::SimpleMemberCommand<QmitkDataStorageFilterProxyModel>::New();
//         command->SetCallbackFunction(this, &QmitkDataStorageFilterProxyModel::UpdateNodeVisibility);
//         m_HelperObjectObserverTags.insert( std::pair<mitk::DataNode*, unsigned long>( const_cast<mitk::DataNode*>(node), node->GetProperty("helper object")->AddObserver( itk::ModifiedEvent(), command ) ) );
//     }
// 
//     if (m_Predicate->CheckNode(node))
//       this->AddNodeInternal(node);
// }


// void QmitkDataStorageFilterProxyModel::RemoveNode( const mitk::DataNode* node )
// {
//     if (node == 0)
//         return;
// 
//     //Removing Observer
//     bool isHelperObject (false);
//     NodeTagMapType::iterator searchIter = m_HelperObjectObserverTags.find( const_cast<mitk::DataNode*>(node) );
//     if (node->GetBoolProperty("helper object", isHelperObject) && searchIter != m_HelperObjectObserverTags.end()) {
//         (*searchIter).first->GetProperty("helper object")->RemoveObserver( (*searchIter).second );
//         m_HelperObjectObserverTags.erase(const_cast<mitk::DataNode*>(node));
//     }
// 
//     this->RemoveNodeInternal(node);
// }


void QmitkDataStorageFilterProxyModel::SetShowHelperObjects(bool _ShowHelperObjects)
{
  m_ShowHelperObjects = _ShowHelperObjects;
  this->UpdateNodeVisibility();
}

void QmitkDataStorageFilterProxyModel::SetShowNodesContainingNoData(bool _ShowNodesContainingNoData)
{
  m_ShowNodesContainingNoData = _ShowNodesContainingNoData;
  this->UpdateNodeVisibility();
}

void QmitkDataStorageFilterProxyModel::UpdateNodeVisibility()
{
  mitk::NodePredicateData::Pointer dataIsNull = mitk::NodePredicateData::New(0);
  mitk::NodePredicateNot::Pointer dataIsNotNull = mitk::NodePredicateNot::New(dataIsNull);// Show only nodes that really contain dat

  if (m_ShowHelperObjects)
  {
    if (m_ShowNodesContainingNoData)
    {
      // Show every node
      m_Predicate = mitk::NodePredicateOr::New(dataIsNull, dataIsNotNull);
    }
    else
    {
      // Show helper objects but not nodes containing no data
      m_Predicate = dataIsNotNull;
    }
  }
  else
  {
      mitk::NodePredicateOr::Pointer isHelperObject =
        mitk::NodePredicateOr::New(
            mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)),
            mitk::NodePredicateProperty::New("hidden object", mitk::BoolProperty::New(true))
            );

    mitk::NodePredicateNot::Pointer isNotHelperObject = mitk::NodePredicateNot::New(isHelperObject);// Show only nodes that are not helper objects
    if (m_ShowNodesContainingNoData)
    {
      // Don't show helper objects but nodes containing no data
      m_Predicate = isNotHelperObject;
    }
    else
    {
      // Don't show helper objects and nodes containing no data
      m_Predicate = mitk::NodePredicateAnd::New(isNotHelperObject, dataIsNotNull);
    }
  }
  this->invalidateFilter();
}

bool QmitkDataStorageFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto model = dynamic_cast<QmitkDataStorageTreeModel*>(this->sourceModel());
    assert(model);

    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    return m_Predicate->CheckNode(model->GetNode(index0));
}


