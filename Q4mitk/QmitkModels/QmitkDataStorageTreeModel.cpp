/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date: 2008-08-13 16:56:36 +0200 (Mi, 13 Aug 2008) $
 Version:   $Revision: 14972 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkDataStorageTreeModel.h"

#include <mitkStringProperty.h>
#include "mitkNodePredicateFirstLevel.h"
#include "mitkNodePredicateAND.h"

QmitkDataStorageTreeModel::QmitkDataStorageTreeModel(mitk::NodePredicateBase* pred, QObject* parent)
 : QAbstractItemModel(parent), m_NodePredicate(pred), m_ViewMode(DSVW_Hirarchy)
{
  SetDataStorage(mitk::DataStorage::GetInstance());
}

QmitkDataStorageTreeModel::~QmitkDataStorageTreeModel()
{
  if (m_NodePredicate) delete m_NodePredicate;
}

void QmitkDataStorageTreeModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;

  this->reset();
}

Qt::ItemFlags QmitkDataStorageTreeModel::flags(const QModelIndex& /*index*/) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  //ItemIsSelectable = 1,
  //ItemIsEditable = 2,
  //ItemIsDragEnabled = 4,
  //ItemIsDropEnabled = 8,
  //ItemIsUserCheckable = 16,
  //ItemIsEnabled = 32,
  //ItemIsTristate = 64
}

QVariant QmitkDataStorageTreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() && index.internalPointer())
		return QVariant();

	mitk::DataTreeNode::Pointer node = static_cast<mitk::DataTreeNode*>(index.internalPointer());

	switch (role)
	{
	case Qt::DisplayRole:
		{
			std::string name = node->GetName();
			return QVariant(QString::fromStdString(name));
		}
		break;
	case QmitkDataTreeNodeRole:
		{
			return QVariant::fromValue(node);
		}
		break;
	default:
		return QVariant();
	}
return 0;
}

QVariant QmitkDataStorageTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant("Scene Data");
}

int QmitkDataStorageTreeModel::rowCount(const QModelIndex& parent) const
{

	if(!parent.isValid())
	{
		if(m_NodePredicate)
		{
      mitk::NodePredicateFirstLevel::Pointer p1 = mitk::NodePredicateFirstLevel::New(m_DataStorage);
			mitk::NodePredicateAND::Pointer p2 = mitk::NodePredicateAND::New(p1,m_NodePredicate);
			return m_DataStorage->GetSubset(p2)->Size();
		}
		else
		{
      mitk::NodePredicateFirstLevel::Pointer p1 = mitk::NodePredicateFirstLevel::New(m_DataStorage);
			int num  = m_DataStorage->GetSubset(p1)->Size();
			return m_DataStorage->GetSubset(p1)->Size();
		}
	}
	else
	{
		mitk::DataTreeNode::Pointer node = static_cast<mitk::DataTreeNode*>(parent.internalPointer());
		return m_DataStorage->GetDerivations(node, m_NodePredicate, true)->Size();
	}
}

QModelIndex QmitkDataStorageTreeModel::parent ( const QModelIndex & index ) const
{
	if (index.isValid())
	{
		mitk::DataTreeNode::Pointer node = 
			data(index, QmitkDataTreeNodeRole).value<mitk::DataTreeNode::Pointer>();
			
		if (m_DataStorage->GetSources(node, m_NodePredicate, true)->Size())
			return createIndex(0, 0, m_DataStorage->GetSources(node, m_NodePredicate, true)->GetElement(0));
		else
			return QModelIndex();
	}

	return QModelIndex();
}

QModelIndex QmitkDataStorageTreeModel::index ( int row, int column, const QModelIndex & parent ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();
	
	if (parent.isValid())
	{
		mitk::DataTreeNode::Pointer node = 
			static_cast<mitk::DataTreeNode*>(parent.internalPointer());
		if (m_DataStorage->GetDerivations(node, m_NodePredicate, true)->GetElement(row))
			return createIndex(row, column, m_DataStorage->GetDerivations(node, m_NodePredicate, true)->GetElement(row));
		else
			return QModelIndex();
	}
	else
	{
		if(m_NodePredicate)
		{
			mitk::NodePredicateFirstLevel::Pointer p1(mitk::NodePredicateFirstLevel::New(m_DataStorage));
			mitk::NodePredicateAND::Pointer p2(mitk::NodePredicateAND::New(p1,m_NodePredicate));
			if (m_DataStorage->GetSubset(p2)->GetElement(row))
				return createIndex(row, column, m_DataStorage->GetSubset(p2)->GetElement(row));
		}
		else
		{
			mitk::NodePredicateFirstLevel::Pointer p1(mitk::NodePredicateFirstLevel::New(m_DataStorage));
			if (m_DataStorage->GetSubset(p1)->Size() > row)
			{
				//int num = m_DataStorage->GetSubset(p1)->Size();
				return createIndex(row, column, m_DataStorage->GetSubset(p1)->GetElement(row));
			}
		}

		return QModelIndex();
	}
}

bool QmitkDataStorageTreeModel::hasChildren ( const QModelIndex & parent ) const
{
	if (parent.isValid())
	{
		mitk::DataTreeNode::Pointer node = static_cast<mitk::DataTreeNode*>(parent.internalPointer());
		return m_DataStorage->GetDerivations(node, m_NodePredicate, true)->Size() > 0;
	}
	if(m_NodePredicate)
	{
		mitk::NodePredicateFirstLevel::Pointer p1(mitk::NodePredicateFirstLevel::New(m_DataStorage));
		mitk::NodePredicateAND::Pointer p2(mitk::NodePredicateAND::New(p1,m_NodePredicate));
		return m_DataStorage->GetSubset(p2)->Size();
	}
	else
	{
		mitk::NodePredicateFirstLevel::Pointer p1(mitk::NodePredicateFirstLevel::New(m_DataStorage));
		return m_DataStorage->GetSubset(p1)->Size();
	}
}

int QmitkDataStorageTreeModel::columnCount ( const QModelIndex & parent ) const
{
	return 1;
}