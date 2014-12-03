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

#include "QmitkPointListModel.h"
#include <itkCommand.h>
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkRenderingManager.h"
#include <mitkPointSetDataInteractor.h>
#include <mitkEvent.h>
#include <mitkStateEvent.h>
#include <mitkInteractionConst.h>
#include <mitkInternalEvent.h>


QmitkPointListModel::QmitkPointListModel( mitk::DataNode* pointSetNode, int t, QObject* parent )
:QAbstractListModel(parent),
m_PointSetNode(NULL),
m_PointSetModifiedObserverTag(0),
m_PointSetDeletedObserverTag(0),
m_TimeStep(t)
{
  ObserveNewPointSet( pointSetNode );
}

Qt::ItemFlags QmitkPointListModel::flags(const QModelIndex& /*index*/) const
{
  // no editing so far, return default (enabled, selectable)
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QmitkPointListModel::~QmitkPointListModel()
{
  this->ObserveNewPointSet( NULL );
}

void QmitkPointListModel::SetPointSetNode( mitk::DataNode* pointSetNode )
{
  this->ObserveNewPointSet( pointSetNode );
  QAbstractListModel::beginResetModel();
  QAbstractListModel::beginResetModel();
  emit SignalUpdateSelection();
}

mitk::PointSet* QmitkPointListModel::GetPointSet() const
{
  return this->CheckForPointSetInNode(m_PointSetNode);
}

void QmitkPointListModel::SetTimeStep(int t)
{
  m_TimeStep = t;
  QAbstractListModel::beginResetModel();
  QAbstractListModel::endResetModel();
  emit SignalUpdateSelection();
}

int QmitkPointListModel::GetTimeStep() const
{
  return m_TimeStep;
}

void QmitkPointListModel::ObserveNewPointSet( mitk::DataNode* pointSetNode )
{

  //remove old observers
  if (m_PointSetNode != NULL)
  {
    try //here sometimes an exception is thrown which leads to a crash. So catch this exception but give an error message. See bug 18316 for details.
      {
      mitk::PointSet::Pointer oldPointSet = dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());
      if (oldPointSet.IsNotNull())
        {
        oldPointSet->RemoveObserver(m_PointSetModifiedObserverTag);
        oldPointSet->RemoveObserver(m_PointSetDeletedObserverTag);
        }
      }
    catch(std::exception& e)
      {
        MITK_ERROR << "Exception while removing observer from old point set node: " << e.what();
      }
  }

  //get the new pointset
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(pointSetNode);

  m_PointSetNode = pointSetNode;

  if ( pointSet.IsNotNull())
  {
    // add new observer for modified if necessary
    itk::ReceptorMemberCommand<QmitkPointListModel>::Pointer modCommand = itk::ReceptorMemberCommand<QmitkPointListModel>::New();
    modCommand->SetCallbackFunction( this, &QmitkPointListModel::OnPointSetChanged );
    m_PointSetModifiedObserverTag = pointSet->AddObserver( itk::ModifiedEvent(), modCommand );

    // add new observer for detele if necessary
    itk::ReceptorMemberCommand<QmitkPointListModel>::Pointer delCommand = itk::ReceptorMemberCommand<QmitkPointListModel>::New();
    delCommand->SetCallbackFunction( this, &QmitkPointListModel::OnPointSetDeleted );
    m_PointSetDeletedObserverTag = pointSet->AddObserver( itk::DeleteEvent(), delCommand );
  }
  else
  {
    m_PointSetModifiedObserverTag = 0;
    m_PointSetDeletedObserverTag = 0;
  }
}

void QmitkPointListModel::OnPointSetChanged(const itk::EventObject&)
{
  QAbstractListModel::beginResetModel();
  QAbstractListModel::endResetModel();
  emit SignalUpdateSelection();
}

void QmitkPointListModel::OnPointSetDeleted(const itk::EventObject&)
{
  mitk::PointSet::Pointer ps = CheckForPointSetInNode(m_PointSetNode);
  if (ps)
  {
    ps->RemoveObserver(m_PointSetModifiedObserverTag);
    ps->RemoveObserver(m_PointSetDeletedObserverTag);
  }

  m_PointSetModifiedObserverTag = 0;
  m_PointSetDeletedObserverTag = 0;
  QAbstractListModel::beginResetModel();
  QAbstractListModel::endResetModel();
}

int QmitkPointListModel::rowCount(const QModelIndex&) const
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if ( pointSet.IsNotNull() )
  {
    return pointSet->GetSize(m_TimeStep);
  }
  else
  {
    return 0;
  }
}

QVariant QmitkPointListModel::data(const QModelIndex& index, int role) const
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);

  if ( pointSet.IsNull() )
  {
    return QVariant();
  }

  if ( !index.isValid() )
  {
    return QVariant();
  }

  if ( index.row() >= pointSet->GetSize(m_TimeStep) )
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    mitk::PointSet::PointsContainer::ElementIdentifier id;
    mitk::PointSet::PointType p;
    bool pointFound = this->GetPointForModelIndex(index, p, id);
    if (pointFound == false)
      return QVariant();

    QString s = QString("%0: (%1, %2, %3)")
      .arg( id, 3)
      .arg( p[0], 0, 'f', 3 )
      .arg( p[1], 0, 'f', 3 )
      .arg( p[2], 0, 'f', 3 );
    return QVariant(s);
  }
  else
  {
    return QVariant();
  }
}

QVariant QmitkPointListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Horizontal)
  {
    return QString("Coordinates").arg(section);
  }
  else
  {
    return QString("Row %1").arg(section);
  }
}

bool QmitkPointListModel::GetPointForModelIndex( const QModelIndex &index, mitk::PointSet::PointType& p,
                                                mitk::PointSet::PointIdentifier& id) const
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if (pointSet.IsNull())
    return false;

  if ((index.row() < 0) || (index.row() >= (int)pointSet->GetPointSet(m_TimeStep)->GetPoints()->Size()))
    return false;

  // get the nth. element, if it exists.
  // we can not use the index directly, because PointSet uses a map container,
  // where the index is not necessarily the same as the key.
  // Therefore we have to count the elements
  mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_TimeStep)->GetPoints()->Begin();
  for (int i = 0; i < index.row(); ++i)
  {
    ++it;

    if (it == pointSet->GetPointSet(m_TimeStep)->GetPoints()->End())
      return false;

  }

  if (it != pointSet->GetPointSet(m_TimeStep)->GetPoints()->End()) // not at the end,
  {
    p = it->Value();
    id = it->Index();
    return true;
  }

  return false;
}

bool QmitkPointListModel::GetModelIndexForPointID(mitk::PointSet::PointIdentifier id, QModelIndex& index) const
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if (pointSet.IsNull())
    return false;

  mitk::PointSet::PointsContainer::Pointer points = pointSet->GetPointSet(m_TimeStep)->GetPoints();

  if (points->IndexExists(id) == false)
    return false;

  unsigned int idx = 0;
  for (mitk::PointSet::PointsContainer::Iterator it = points->Begin(); it != points->End(); ++it)
  {
    if (it->Index() == id) // we found the correct element
    {
      index = this->index(idx);
      return true;
    }

    idx++;
  }

  return false; // nothing found
}

void QmitkPointListModel::MoveSelectedPointUp()
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if (pointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::ScalarType tsInMS = pointSet->GetTimeGeometry()->TimeStepToTimePoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP,tsInMS, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkPointListModel::MoveSelectedPointDown()
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if (pointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::ScalarType tsInMS = pointSet->GetTimeGeometry()->TimeStepToTimePoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, tsInMS, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}


void QmitkPointListModel::RemoveSelectedPoint()
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if (pointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::ScalarType tsInMS = pointSet->GetTimeGeometry()->TimeStepToTimePoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, tsInMS, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

mitk::PointSet* QmitkPointListModel::CheckForPointSetInNode(mitk::DataNode* node) const
{
  if (node != NULL)
  {
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    if (pointSet.IsNotNull())
      return pointSet;
  }
  return NULL;
}

