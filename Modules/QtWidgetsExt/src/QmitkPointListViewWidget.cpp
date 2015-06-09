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

#include "QmitkPointListViewWidget.h"

#include "QmitkPointListModel.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkEditPointDialog.h"

#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkRenderingManager.h"

#include <QKeyEvent>

QmitkPointListViewWidget::QmitkPointListViewWidget( QWidget* parent )
:QListWidget( parent ),
 m_TimeStep( 0 ),
 m_SelfCall( false ),
 m_MultiWidget( NULL)
{
  QListWidget::setAlternatingRowColors( true );
  // logic

  QListWidget::setSelectionBehavior( QAbstractItemView::SelectRows );
  QListWidget::setSelectionMode( QAbstractItemView::SingleSelection );

  connect( this, SIGNAL(itemDoubleClicked ( QListWidgetItem * )),
           this, SLOT(OnItemDoubleClicked( QListWidgetItem *)) );

  connect( this, SIGNAL( currentRowChanged( int ) ),
           this, SLOT( OnCurrentRowChanged( int ) ) );

}

QmitkPointListViewWidget::~QmitkPointListViewWidget()
{
  this->SetPointSet(0); // remove listener
}

void QmitkPointListViewWidget::SetPointSet( mitk::PointSet* pointSet )
{
  if(m_PointSet.IsNotNull())
  {
      m_PointSet.ObjectModified.RemoveListener
        (mitk::MessageDelegate1<QmitkPointListViewWidget
          , const itk::Object*>( this, &QmitkPointListViewWidget::OnPointSetChanged ));
      m_PointSet.ObjectDelete.RemoveListener
        (mitk::MessageDelegate1<QmitkPointListViewWidget
          , const itk::Object*>( this, &QmitkPointListViewWidget::OnPointSetDeleted ));
  }

  m_PointSet = pointSet;

  if(m_PointSet.IsNotNull())
  {
      m_PointSet.ObjectModified.AddListener
        (mitk::MessageDelegate1<QmitkPointListViewWidget
          , const itk::Object*>( this, &QmitkPointListViewWidget::OnPointSetChanged ));
      m_PointSet.ObjectDelete.AddListener
        (mitk::MessageDelegate1<QmitkPointListViewWidget
          , const itk::Object*>( this, &QmitkPointListViewWidget::OnPointSetDeleted ));
  }

  this->Update();
}

const mitk::PointSet* QmitkPointListViewWidget::GetPointSet() const
{
  return m_PointSet;
}

void QmitkPointListViewWidget::SetTimeStep(int t)
{
  m_TimeStep = t;
  this->Update();
}

int QmitkPointListViewWidget::GetTimeStep() const
{
  return m_TimeStep;
}

void QmitkPointListViewWidget::SetMultiWidget( QmitkStdMultiWidget* multiWidget )
{
  m_MultiWidget = multiWidget;
}


QmitkStdMultiWidget* QmitkPointListViewWidget::GetMultiWidget() const
{
  return m_MultiWidget;
}

void QmitkPointListViewWidget::OnPointSetChanged(const itk::Object*)
{
  if(!m_SelfCall)
    this->Update();
}

void QmitkPointListViewWidget::OnPointSetDeleted(const itk::Object*)
{
  this->SetPointSet(0);
  this->Update();
}

void QmitkPointListViewWidget::OnItemDoubleClicked(QListWidgetItem * item)
{
  QmitkEditPointDialog _EditPointDialog(this);
  _EditPointDialog.SetPoint(m_PointSet, this->row(item), m_TimeStep);
  _EditPointDialog.exec();
}

void QmitkPointListViewWidget::OnCurrentRowChanged(int)
{
  this->Update(true);
}

void QmitkPointListViewWidget::keyPressEvent( QKeyEvent * e )
{
  if (m_PointSet.IsNull())
    return;

  int key = e->key();
  switch (key)
  {
    case Qt::Key_F2:
     this->MoveSelectedPointUp();
     break;
    case Qt::Key_F3:
      this->MoveSelectedPointDown();
      break;
    case Qt::Key_Delete:
      this->RemoveSelectedPoint();
      break;
    default:
    break;
  }
}

void QmitkPointListViewWidget::MoveSelectedPointUp()
{
  if (m_PointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}


void QmitkPointListViewWidget::MoveSelectedPointDown()
{
  if (m_PointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}


void QmitkPointListViewWidget::RemoveSelectedPoint()
{
  if (m_PointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = m_PointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, m_PointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  m_PointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkPointListViewWidget::Update(bool currentRowChanged)
{
  if(m_SelfCall)
    return;

  if(m_PointSet.IsNull())
  {
    this->clear();
    return;
  }

  m_SelfCall = true;
  QString text;
  int i = 0;

  mitk::PointSet::DataType::Pointer pointset = m_PointSet->GetPointSet(m_TimeStep);
  for (mitk::PointSet::PointsContainer::Iterator it = pointset->GetPoints()->Begin(); it != pointset->GetPoints()->End(); ++it)
  {

    text = QString("%0: (%1, %2, %3)")
      .arg( i, 3)
      .arg( it.Value().GetElement(0), 0, 'f', 3 )
      .arg( it.Value().GetElement(1), 0, 'f', 3 )
      .arg( it.Value().GetElement(2), 0, 'f', 3 );

    if(i==this->count())
      this->addItem(text); // insert text
    else
      this->item(i)->setText(text); // update text

    if(currentRowChanged)
    {
      if(i == this->currentRow())
        m_PointSet->SetSelectInfo(this->currentRow(), true, m_TimeStep);
      else
        m_PointSet->SetSelectInfo(it->Index(), false, m_TimeStep); // select nothing now
    }
    ++i;
  }

  // remove unnecessary listwidgetitems
  while (m_PointSet->GetPointSet(m_TimeStep)->GetPoints()->Size() < (unsigned int)this->count() )
  {
    QListWidgetItem * item = this->takeItem(this->count()-1);
    delete item;
  }

  // update selection in pointset or in the list widget
  if(!currentRowChanged)
  {
    if ( m_PointSet->GetNumberOfSelected( m_TimeStep ) > 1 )
    {
      /// @TODO use logging as soon as available
      std::cerr << "Point set has multiple selected points. This view is not designed for more than one selected point." << std::endl;
    }

    int selectedIndex = m_PointSet->SearchSelectedPoint( m_TimeStep );
    if (selectedIndex != -1) // no selected point is found
    {
      this->setCurrentRow ( selectedIndex );
    }
  }
  m_SelfCall = false;
}

