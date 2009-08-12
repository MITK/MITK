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

#include "QmitkPointListView.h"

#include "QmitkPointListModel.h"
#include "QmitkStdMultiWidget.h"

#include "mitkRenderingManager.h"

QmitkPointListView::QmitkPointListView( QWidget* parent )
:QListView( parent ),
 m_PointListModel( new QmitkPointListModel() ),
 m_SelfCall( false ),
 m_MultiWidget( NULL)
{
  // cosmetics
  QListView::setAlternatingRowColors( true );
  
  // logic
  QListView::setModel( m_PointListModel );

  QListView::setSelectionBehavior( QAbstractItemView::SelectRows );
  QListView::setSelectionMode( QAbstractItemView::SingleSelection );

  connect( m_PointListModel, SIGNAL(UpdateSelection()), this, SLOT(OnPointSetSelectionChanged()) );
  connect( QListView::selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
           this, SLOT(OnListViewSelectionChanged(const QItemSelection& , const QItemSelection&)) );
}

QmitkPointListView::~QmitkPointListView()
{
  delete m_PointListModel;
}

void QmitkPointListView::SetPointSet( mitk::PointSet* pointSet )
{
  m_PointListModel->SetPointSet( pointSet );
}

const mitk::PointSet* QmitkPointListView::GetPointSet() const
{
  return m_PointListModel->GetPointSet();
}
     
void QmitkPointListView::SetMultiWidget( QmitkStdMultiWidget* multiWidget )
{
  m_MultiWidget = multiWidget;
}


QmitkStdMultiWidget* QmitkPointListView::GetMultiWidget() const
{
  return m_MultiWidget;
}


void QmitkPointListView::OnPointSetSelectionChanged()
{
  if (m_SelfCall) return;

  m_SelfCall = true;

  // update this view's selection status as a result to changes in the point set data structure
  //std::cout << "update view selection from point set" << std::endl;
  const mitk::PointSet* pointSet = m_PointListModel->GetPointSet();
  if (pointSet)
  {
    int timeStep = m_PointListModel->GetTimeStep();

    if ( pointSet->GetNumberOfSelected( timeStep ) > 1 )
    {
      /// @TODO use logging as soon as available
      std::cerr << "Point set has multiple selected points. This view is not designed for more than one selected point." << std::endl;
    }

    int selectedIndex = pointSet->SearchSelectedPoint( timeStep );

    QListView::selectionModel()->select( m_PointListModel->index( selectedIndex ),
                                         QItemSelectionModel::SelectCurrent );
  }
  
  m_SelfCall = false;
}

void QmitkPointListView::OnListViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  if (m_SelfCall) return;

  mitk::PointSet* pointSet = const_cast<mitk::PointSet*>( m_PointListModel->GetPointSet() );
  int timeStep = m_PointListModel->GetTimeStep();

  if (!pointSet) return;

  // select point in point set
  // (take care that this widget doesn't react to self-induced changes by setting m_SelfCall)
  m_SelfCall = true;

  QModelIndexList selectedIndexes = selected.indexes();
  for (int i = 0; i < selectedIndexes.size(); ++i) 
  {
    QModelIndex index = selectedIndexes.at(i);
    if ( index.isValid() )
    {
      int selectedIndex = index.row();
      pointSet->SetSelectInfo( selectedIndex, true, timeStep );
      //std::cout << "  select row " << selectedIndex << std::endl;

      // move crosshair to selected point
      if ( m_MultiWidget )
      {
        m_MultiWidget->MoveCrossToPosition( pointSet->GetPoint( selectedIndex, timeStep ) );
      }
    }
  }

  QModelIndexList deselectedIndexes = deselected.indexes();
  for (int i = 0; i < deselectedIndexes.size(); ++i) 
  {
    QModelIndex index = deselectedIndexes.at(i);
    if ( index.isValid() )
    {
      int selectedIndex = index.row();
      pointSet->SetSelectInfo( selectedIndex, false, timeStep );
      //std::cout << "  deselect row " << selectedIndex << std::endl;
    }
  }
  m_SelfCall = false;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
