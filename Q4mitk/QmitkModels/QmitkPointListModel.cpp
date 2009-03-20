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
    
QmitkPointListModel::QmitkPointListModel( const mitk::PointSet* pointSet, int t, QObject* parent )
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
    
void QmitkPointListModel::SetPointSet( const mitk::PointSet* pointSet )
{
  ObserveNewPointset( pointSet );
  emit QAbstractListModel::layoutChanged();
  emit UpdateSelection();
}
    
const mitk::PointSet* QmitkPointListModel::GetPointSet()
{
  return m_PointSet;
}
    
void QmitkPointListModel::SetTimeStep(int t)
{
  m_TimeStep = t;
  emit QAbstractListModel::layoutChanged();
  emit UpdateSelection();
}
    
int QmitkPointListModel::GetTimeStep()
{
  return m_TimeStep;
}

void QmitkPointListModel::ObserveNewPointset( const mitk::PointSet* pointSet )
{
  // remove old observer
  if ( m_PointSet != NULL )
  {
    const_cast<mitk::PointSet*>(m_PointSet)->RemoveObserver( m_PointSetModifiedObserverTag );
    const_cast<mitk::PointSet*>(m_PointSet)->RemoveObserver( m_PointSetDeletedObserverTag );
  }

  m_PointSet = pointSet;

  // add new observer for modified if neccessary
  if ( m_PointSet != NULL )
  {
    itk::ReceptorMemberCommand<QmitkPointListModel>::Pointer command = itk::ReceptorMemberCommand<QmitkPointListModel>::New();
    command->SetCallbackFunction( this, &QmitkPointListModel::OnPointSetChanged );
    m_PointSetModifiedObserverTag = const_cast<mitk::PointSet*>(m_PointSet)->AddObserver( itk::ModifiedEvent(), command );
  }
  else
  {
    m_PointSetModifiedObserverTag = 0;
  }

  // add new observer for modified if neccessary
  if ( m_PointSet != NULL )
  {
    itk::ReceptorMemberCommand<QmitkPointListModel>::Pointer command = itk::ReceptorMemberCommand<QmitkPointListModel>::New();
    command->SetCallbackFunction( this, &QmitkPointListModel::OnPointSetDeleted );
    m_PointSetDeletedObserverTag = const_cast<mitk::PointSet*>(m_PointSet)->AddObserver( itk::DeleteEvent(), command );
  }
  else
  {
    m_PointSetDeletedObserverTag = 0;
  }
}
    
void QmitkPointListModel::OnPointSetChanged( const itk::EventObject & e )
{
  //std::cout << "point set changed" << std::endl;
  emit QAbstractListModel::layoutChanged();
  emit UpdateSelection();
}

void QmitkPointListModel::OnPointSetDeleted( const itk::EventObject & e )
{
  m_PointSet = NULL;
  m_PointSetModifiedObserverTag = 0;
  m_PointSetDeletedObserverTag = 0;

  //std::cout << "point set deleted" << std::endl;
  emit QAbstractListModel::layoutChanged();
}


int QmitkPointListModel::rowCount( const QModelIndex& parent ) const
{
  if ( m_PointSet != NULL )
  {
    return m_PointSet->GetSize();
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
    mitk::Point3D p = m_PointSet->GetPoint( index.row() );
    QString s = QString("(%1, %2, %3)")
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

