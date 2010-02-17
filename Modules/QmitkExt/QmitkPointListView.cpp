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

#include <QKeyEvent>

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

  //Define Size
  this->setMinimumHeight(40);
  //horizontal, vertical
  this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

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
  if (m_SelfCall) 
    return;


  const mitk::PointSet* pointSet = m_PointListModel->GetPointSet();
  if (pointSet == NULL)
    return;

  // update this view's selection status as a result to changes in the point set data structure
  m_SelfCall = true;
  int timeStep = m_PointListModel->GetTimeStep();

  if ( pointSet->GetNumberOfSelected( timeStep ) > 1 )
  {
    /// @TODO use logging as soon as available
    std::cerr << "Point set has multiple selected points. This view is not designed for more than one selected point." << std::endl;
  }

  int selectedIndex = pointSet->SearchSelectedPoint( timeStep );
  if (selectedIndex == -1) // no selected point is found
  {
    m_SelfCall = false;
    return;
  }
  QModelIndex index;
  bool modelIndexOkay = m_PointListModel->GetModelIndexForPointID(selectedIndex, index);
  if (modelIndexOkay == true)
    QListView::selectionModel()->select( m_PointListModel->index( selectedIndex ), QItemSelectionModel::SelectCurrent );
 
  emit PointSelectionChanged();
  m_SelfCall = false;
}


void QmitkPointListView::OnListViewSelectionChanged(const QItemSelection& selected, const QItemSelection&  /*deselected*/)
{
  if (m_SelfCall) 
    return;

  mitk::PointSet* pointSet = const_cast<mitk::PointSet*>( m_PointListModel->GetPointSet() );
  //int timeStep = m_PointListModel->GetTimeStep();

  if (pointSet == NULL) 
    return;

  // (take care that this widget doesn't react to self-induced changes by setting m_SelfCall)
  m_SelfCall = true;

  /* update selection of all points in pointset: select the one(s) that are selected in the view, deselect all others */
  QModelIndexList selectedIndexes = selected.indexes();
  for (mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_PointListModel->GetTimeStep())->GetPoints()->Begin(); it != pointSet->GetPointSet(m_PointListModel->GetTimeStep())->GetPoints()->End(); ++it)
  {
    QModelIndex index;
    m_PointListModel->GetModelIndexForPointID(it->Index(), index);
    if (selectedIndexes.indexOf(index) != -1) // index is found in the selected indices list
    {
      pointSet->SetSelectInfo(it->Index(), true, m_PointListModel->GetTimeStep());
      if ( m_MultiWidget != NULL)
        m_MultiWidget->MoveCrossToPosition(pointSet->GetPoint(it->Index(), m_PointListModel->GetTimeStep()));        
     
    }
    else
      pointSet->SetSelectInfo(it->Index(), false, m_PointListModel->GetTimeStep());
  }
  m_SelfCall = false;
  emit PointSelectionChanged();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPointListView::keyPressEvent( QKeyEvent * e )
{
  if (m_PointListModel == NULL)
    return;

  int key = e->key();
  switch (key)
  {
    case Qt::Key_F2:
     m_PointListModel->MoveSelectedPointUp();
     break;
    case Qt::Key_F3:
      m_PointListModel->MoveSelectedPointDown();
      break;
    case Qt::Key_Delete:
      m_PointListModel->RemoveSelectedPoint();
      break;
    default:
    break;
  }
}
