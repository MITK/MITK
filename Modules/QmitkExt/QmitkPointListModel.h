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

#ifndef QMITK_POINTLIST_MODEL_H_INCLUDED
#define QMITK_POINTLIST_MODEL_H_INCLUDED

#include <QAbstractListModel>

#include "mitkPointSet.h"

class QMITKEXT_EXPORT QmitkPointListModel : public QAbstractListModel
{
  Q_OBJECT

  public:

    QmitkPointListModel( const mitk::PointSet* = NULL, int t = 0, QObject* parent = 0 );
    ~QmitkPointListModel();

    // interface of QAbstractListModel
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;

    // interface of QAbstractListModel
    QVariant data(const QModelIndex& index, int role) const;
         
    // interface of QAbstractListModel
    QVariant headerData(int section, 
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /// which point set to work on
    void SetPointSet( const mitk::PointSet* pointSet );
    
    /// which point set to work on
    const mitk::PointSet* GetPointSet();

    /// which time step to display/model
    void SetTimeStep(int t);
    
    /// which time step to display/model
    int GetTimeStep();

    /// itk observer for point set "modified" events
    void OnPointSetChanged( const itk::EventObject & e );
    
    /// itk observer for point set "delete" events
    void OnPointSetDeleted( const itk::EventObject & e );

  signals:

    /// emitted, when views should update their selection status
    /// (because mouse interactions in render windows can change
    /// the selection status of points)
    void UpdateSelection();

  protected:

    /// internally observe different point set
    void ObserveNewPointset( const mitk::PointSet* pointSet );

    const mitk::PointSet* m_PointSet;
    unsigned int m_PointSetModifiedObserverTag;
    unsigned int m_PointSetDeletedObserverTag;
    
    int m_TimeStep;
};

#endif

