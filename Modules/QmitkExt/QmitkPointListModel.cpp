/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPointListModel.h"
#include <itkCommand.h>
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkRenderingManager.h"


QmitkPointListModel::QmitkPointListModel( mitk::PointSet* pointSet, int t, QObject* parent )
:QAbstractListModel(parent),
m_PointSet(NULL),
m_PointSetModifiedObserverTag(0),
m_PointSetDeletedObserverTag(0),
m_TimeStep(t)
{
  ObserveNewPointset( pointSet );

  
}


QmitkPointListModel::~QmitkPointListModel()
{
  ObserveNewPointset( NULL );
}


void QmitkPointListModel::SetPointSet( mitk::PointSet* pointSet )
{
  ObserveNewPointset( pointSet );
  emit QAbstractListModel::layoutChanged();
  emit UpdateSelection();
}


mitk::PointSet* QmitkPointListModel::GetPointSet() const
{
  return m_PointSet;
}


void QmitkPointListModel::SetTimeStep(int t)
{
  m_TimeStep = t;
  emit QAbstractListModel::layoutChanged();
  emit UpdateSelection();
}


int QmitkPointListModel::GetTimeStep() const
{
  return m_TimeStep;
}


void QmitkPointListModel::ObserveNewPointset( mitk::PointSet* pointSet )
{
  // remove old observer
  if ( m_PointSet != NULL )
  {
    m_PointSet->RemoveObserver( m_PointSetModifiedObserverTag );
    m_PointSet->RemoveObserver( m_PointSetDeletedObserverTag );
  }

  m_PointSet = pointSet;

  // add new observer for modified if necessary
  if ( m_PointSet != NULL )
  {
    itk::ReceptorMemberCommand<QmitkPointListModel>::Pointer command = itk::ReceptorMemberCommand<QmitkPointListModel>::New();
    command->SetCallbackFunction( this, &QmitkPointListModel::OnPointSetChanged );
    m_PointSetModifiedObserverTag = m_PointSet->AddObserver( itk::ModifiedEvent(), command );
  }
  else
  {
    m_PointSetModifiedObserverTag = 0;
  }

  // add new observer for modified if necessary
  if ( m_PointSet != NULL )
  {
    itk::ReceptorMemberCommand<QmitkPointListModel>::Pointer command = itk::ReceptorMemberCommand<QmitkPointListModel>::New();
    command->SetCallbackFunction( this, &QmitkPointListModel::OnPointSetDeleted );
    m_PointSetDeletedObserverTag = m_PointSet->AddObserver( itk::DeleteEvent(), command );
  }
  else
  {
    m_PointSetDeletedObserverTag = 0;
  }
}


void QmitkPointListModel::OnPointSetChanged( const itk::EventObject &  /*e*/ )
{
  this->reset();
  emit QAbstractListModel::layoutChanged();
  emit UpdateSelection();
}


void QmitkPointListModel::OnPointSetDeleted( const itk::EventObject &  /*e*/ )
{
  m_PointSet = NULL;
  m_PointSetModifiedObserverTag = 0;
  m_PointSetDeletedObserverTag = 0;
  emit QAbstractListModel::layoutChanged();
}


int QmitkPointListModel::rowCount( const QModelIndex&  /*parent*/ ) const
{
  if ( m_PointSet != NULL )
  {
    return m_PointSet->GetSize(m_TimeStep);
  }
  else
  {
    return 0;
  }
}


QVariant QmitkPointListModel::data(const QModelIndex& index, int role) const
{
  if ( m_PointSet == NULL )
  {
    return QVariant();
  }

  if ( !index.isValid() )
  {
    return QVariant();
  }

  if ( index.row() >= m_PointSet->GetSize(m_TimeStep) )
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


bool QmitkPointListModel::GetPointForModelIndex( const QModelIndex &index, mitk::PointSet::PointType& p, mitk::PointSet::PointIdentifier& id) const
{
  if (m_PointSet == NULL)
    return false;

  if ((index.row() < 0) || (index.row() >= m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->Size()))
    return false;

  // get the nth. element, if it exists.
  // we can not use the index directly, because PointSet uses a map container, 
  // where the index is not necessarily the same as the key.
  // Therefore we have to count the elements
  mitk::PointSet::PointsContainer::Iterator it = m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->Begin();
  for (int i = 0; i < index.row(); ++i)
  {
    ++it;
    if (it == m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->End())
      return false;
  }
  if (it != m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->End()) // not at the end, 
  {
    p = it->Value();
    id = it->Index();
    return true;
  }
  return false;
}


bool QmitkPointListModel::GetModelIndexForPointID(mitk::PointSet::PointIdentifier id, QModelIndex& index) const
{
  if (m_PointSet == NULL)
    return false;

  if (m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->IndexExists(id) == false)
    return false;

  unsigned int idx = 0;
  for (mitk::PointSet::PointsContainer::Iterator it = m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->Begin(); it != m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->End(); ++it)
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


void QmitkPointListModel::SelectSelectedPoint()
{
  if(m_PointSet == NULL)
    return;

  mitk::PointSet::PointIdentifier selectedID; 
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpSELECTPOINT, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkPointListModel::MoveSelectedPointUp()
{
  if (m_PointSet == NULL)
    return;
  
  mitk::PointSet::PointIdentifier selectedID; 
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}


void QmitkPointListModel::MoveSelectedPointDown()
{
  if (m_PointSet == NULL)
    return;

  mitk::PointSet::PointIdentifier selectedID; 
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}


void QmitkPointListModel::RemoveSelectedPoint()
{
  if (m_PointSet == NULL)
    return;

  mitk::PointSet::PointIdentifier selectedID; 
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}
