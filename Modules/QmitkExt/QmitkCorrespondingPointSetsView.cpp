/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkCorrespondingPointSetsView.h"

#include "QmitkCorrespondingPointSetsModel.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkEditPointDialog.h"

#include "mitkRenderingManager.h"

#include <mitkStepper.h>
#include <QKeyEvent>
#include <QPalette>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject.h>
#include <QMetaProperty>
#include <mitkStepper.h>
#include <mitkPointSetInteractor.h>
#include <mitkGlobalInteraction.h>

QmitkCorrespondingPointSetsView::QmitkCorrespondingPointSetsView( QWidget* parent )
  :QTableView( parent ),
  m_CorrespondingPointSetsModel( new QmitkCorrespondingPointSetsModel() ),
  m_SelfCall( false ),
  m_swapPointSets(false),
  m_addPointsMode(false),
  m_MultiWidget( NULL ),
  m_Interactor(NULL),
  m_TimeStepper( NULL )
{  
  // logic
  m_CorrespondingPointSetsModel->setProperty("QTPropShowCoordinates", true);
  m_CorrespondingPointSetsModel->setProperty("QTPropShowIds", false);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  QTableView::setAlternatingRowColors( true );
  QTableView::setSelectionBehavior( QAbstractItemView::SelectItems );
  QTableView::setSelectionMode( QAbstractItemView::SingleSelection );
  QTableView::setModel( m_CorrespondingPointSetsModel );
  //QTableView::horizontalHeader()->resizeSections(QHeaderView::Stretch);
  QTableView::horizontalHeader()->resizeSection(0, (int)(this->width()/3.5));
  QTableView::horizontalHeader()->setStretchLastSection(true);
  /*this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setDropIndicatorShown(true);
  this->setDragDropMode(QAbstractItemView::InternalMove);*/
  m_TimeStepFaderLabel = new QLabel(this);
  QFont font("Arial", 17);
  m_TimeStepFaderLabel->setFont(font);

  //connect
  connect( m_CorrespondingPointSetsModel, SIGNAL(SignalUpdateSelection()), this, SLOT(OnPointSelectionChanged()) );

  connect( QTableView::selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
           this, SLOT(OnPointSelectionChanged(const QItemSelection& , const QItemSelection&)) );

  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenu(const QPoint &)));
}

QmitkCorrespondingPointSetsView::~QmitkCorrespondingPointSetsView()
{
  delete m_CorrespondingPointSetsModel;
}
void QmitkCorrespondingPointSetsView::SetPointSetNodes( std::vector<mitk::DataNode*> nodes )
{
  if ( !this->m_swapPointSets || nodes.size()<2 )
    m_CorrespondingPointSetsModel->SetPointSetNodes( nodes );
  else
  {
    std::vector<mitk::DataNode*> reverseNodes;
    reverseNodes.push_back(nodes.back());
    reverseNodes.push_back(nodes.front());
    m_CorrespondingPointSetsModel->SetPointSetNodes( reverseNodes );
  }
}
void QmitkCorrespondingPointSetsView::SetMultiWidget( QmitkStdMultiWidget* multiWidget )
{
  m_MultiWidget = multiWidget;
  m_TimeStepper = m_MultiWidget->GetTimeNavigationController()->GetTime();
  this->m_CorrespondingPointSetsModel->SetStepper(m_TimeStepper);
}
QmitkStdMultiWidget* QmitkCorrespondingPointSetsView::GetMultiWidget() const
{
  return m_MultiWidget;
}

void QmitkCorrespondingPointSetsView::OnPointSelectionChanged(const QItemSelection& selected, const QItemSelection&  /*deselected*/)
{
  if (m_SelfCall)
    return;

  //mitk::PointSet* pointSet = const_cast<mitk::PointSet*>( m_CorrespondingPointSetsModel->GetPointSet() );
  std::vector<mitk::DataNode*> pointSetNodes = this->GetPointSetNodes();

  // (take care that this widget doesn't react to self-induced changes by setting m_SelfCall)
  m_SelfCall = true;

  QModelIndexList selectedIndexes = selected.indexes();
  if (selectedIndexes.size() > 0)
  {
    QModelIndex index = selectedIndexes.front();
    mitk::DataNode* pointSetNode = NULL;
    mitk::PointSet* pointSet = NULL;

    if (index.column() == 0)
    {
      pointSetNode = pointSetNodes.front();
    }
    else
    {
      pointSetNode = pointSetNodes.back();
    }

    if (pointSetNode)
    {
      pointSet = dynamic_cast<mitk::PointSet*>(pointSetNode->GetData());

      for (mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_CorrespondingPointSetsModel->GetTimeStep())->GetPoints()->Begin();
        it != pointSet->GetPointSet(m_CorrespondingPointSetsModel->GetTimeStep())->GetPoints()->End(); ++it)
        {
          QModelIndex tempIndex;
          if (m_CorrespondingPointSetsModel->GetModelIndexForPointID(it->Index(), tempIndex, index.column()))
          {
            if (tempIndex == index)
            {
              pointSet->SetSelectInfo(it->Index(), true, m_CorrespondingPointSetsModel->GetTimeStep());
              
              m_CorrespondingPointSetsModel->SetSelectedIndex(index.row(), index.column());
              if ( m_MultiWidget != NULL)
              {
                m_MultiWidget->MoveCrossToPosition(pointSet->GetPoint(it->Index(), m_CorrespondingPointSetsModel->GetTimeStep()));
              }
            }
            else
            {
              pointSet->SetSelectInfo(it->Index(), false, m_CorrespondingPointSetsModel->GetTimeStep());
            }
          }
        }
    }
  }
  
  m_SelfCall = false;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkCorrespondingPointSetsView::keyPressEvent( QKeyEvent * e )
{
  if (m_CorrespondingPointSetsModel == NULL)
    return;

  int key = e->key();

  switch (key)
  {
  case Qt::Key_F2:
    m_CorrespondingPointSetsModel->MoveSelectedPointUp();
    break;
  case Qt::Key_F3:
    m_CorrespondingPointSetsModel->MoveSelectedPointDown();
    break;
  case Qt::Key_Delete:
    m_CorrespondingPointSetsModel->RemoveSelectedPoint();
    break;
  default:
    break;
  }
}

void QmitkCorrespondingPointSetsView::wheelEvent(QWheelEvent *event)
{
  int whe = event->delta();
  int pos = m_TimeStepper->GetPos();

  int currentTS =  this->m_CorrespondingPointSetsModel->GetTimeStep();
  if(whe > 0)
  {
    this->m_CorrespondingPointSetsModel->SetTimeStep(++currentTS);
    m_TimeStepper->SetPos(++pos);
  }
  else if( pos > 0 )
  {
    this->m_CorrespondingPointSetsModel->SetTimeStep(--currentTS);
    m_TimeStepper->SetPos(--pos);
  }
  fadeTimeStepIn();
}

void QmitkCorrespondingPointSetsView::fadeTimeStepIn()
{
  //Setup Widget
  QWidget *m_TimeStepFader = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(m_TimeStepFader);

  int x = (int)(this->geometry().x()+this->width()*0.75);
  int y = (int)(this->geometry().y()+this->height()*0.75);
  m_TimeStepFader->move(x,y);
  m_TimeStepFader->resize(60, 55);
  m_TimeStepFader->setLayout(layout);
  m_TimeStepFader->setAttribute(Qt::WA_DeleteOnClose);

  layout->addWidget(m_TimeStepFaderLabel);
  m_TimeStepFaderLabel->setAlignment(Qt::AlignCenter);
  m_TimeStepFaderLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  m_TimeStepFaderLabel->setLineWidth(2);
  m_TimeStepFaderLabel->setText(QString("%1").arg(this->m_TimeStepper->GetPos()));

  //give the widget opacity and some colour
  QPalette pal = m_TimeStepFaderLabel->palette();
  QColor semiTransparentColor(139, 192, 223, 50);
  QColor labelTransparentColor(0,0,0,200);
  pal.setColor(m_TimeStepFaderLabel->backgroundRole(), semiTransparentColor);
  pal.setColor(m_TimeStepFaderLabel->foregroundRole(), labelTransparentColor);
  m_TimeStepFaderLabel->setAutoFillBackground(true);
  m_TimeStepFaderLabel->setPalette(pal);

  //show the widget
  m_TimeStepFader->show();

  //and start the timer
  m_TimeStepFaderLabel->setVisible(true);
  QTimer::singleShot(2000, this, SLOT(fadeTimeStepOut()));
}
void QmitkCorrespondingPointSetsView::fadeTimeStepOut()
{
  m_TimeStepFaderLabel->hide();
}
void QmitkCorrespondingPointSetsView::ctxMenu(const QPoint &pos)
{
  QMenu *menu = new QMenu;
  int x = pos.x();
  int y = pos.y();

  int row = QTableView::rowAt(y);
  int col = QTableView::columnAt(x);

  //add delete point action
  mitk::PointSet::PointsContainer::ElementIdentifier id;
  mitk::PointSet::PointType p;
  if (m_CorrespondingPointSetsModel->GetPointForModelIndex(row, col, p, id))
  {
    QAction *movePointUp = new QAction(this);
    movePointUp->setText("Move point up");
    connect(movePointUp, SIGNAL(triggered()), this, SLOT(MoveSelectedPointUp()));
    menu->addAction(movePointUp);

    QAction *movePointDown = new QAction(this);
    movePointDown->setText("Move point down");
    connect(movePointDown, SIGNAL(triggered()), this, SLOT(MoveSelectedPointDown()));
    menu->addAction(movePointDown);

    QAction *delPoint = new QAction(this);
    delPoint->setText("Delete point");
    connect(delPoint, SIGNAL(triggered()), this, SLOT(DeleteSelectedPoint()));
    menu->addAction(delPoint);
  }

  QAction *separator = new QAction(this);
  separator->setSeparator(true);

  if (col == 0 || col == 1)
  {
    menu->addSeparator();

    QAction *clearTS = new QAction(this);
    clearTS->setText("Clear time step");
    connect(clearTS, SIGNAL(triggered()), this, SLOT(ClearCurrentTimeStep()));
    menu->addAction(clearTS);

    QAction *clearList = new QAction(this);
    clearList->setText("Clear point set");
    connect(clearList, SIGNAL(triggered()), this, SLOT(ClearSelectedPointSet()));
    menu->addAction(clearList);
  }
  menu->addSeparator();

  QAction *swapSets = new QAction(this);
  swapSets->setText("Swap point sets");
  connect(swapSets, SIGNAL(triggered(bool)), this, SLOT(SwapPointSets(bool)));
  swapSets->setCheckable(true);
  swapSets->setChecked(m_swapPointSets);
  menu->addAction(swapSets);

  QAction *addPoints = new QAction(this);
  addPoints->setText("Check to add new points");
  connect(addPoints, SIGNAL(triggered(bool)), this, SLOT(AddPointsMode(bool)));
  addPoints->setCheckable(true);
  addPoints->setChecked(m_addPointsMode);
  menu->addAction(addPoints);

  menu->exec(this->mapToGlobal(pos));
}

std::vector<mitk::DataNode*> QmitkCorrespondingPointSetsView::GetPointSetNodes(){
  return this->m_CorrespondingPointSetsModel->GetPointSetNodes();
}
std::vector<mitk::PointSet*> QmitkCorrespondingPointSetsView::GetPointSets(){
  std::vector<mitk::DataNode*> pointSetNodes = GetPointSetNodes();
  std::vector<mitk::PointSet*> pointSets;

  std::vector<mitk::DataNode*>::iterator it;
  for ( it = pointSetNodes.begin(); it < pointSetNodes.end(); it++ )
  {
    mitk::PointSet* pointSet = NULL;
    pointSet = dynamic_cast<mitk::PointSet*> ( dynamic_cast<mitk::DataNode*>(*it)->GetData() );
    if ( pointSet != NULL )
      pointSets.push_back(pointSet);
  }

  return pointSets;
}
void QmitkCorrespondingPointSetsView::DeleteSelectedPoint()
{
  this->m_CorrespondingPointSetsModel->RemoveSelectedPoint();
}
void QmitkCorrespondingPointSetsView::MoveSelectedPointDown()
{
  this->m_CorrespondingPointSetsModel->MoveSelectedPointDown();
}
void QmitkCorrespondingPointSetsView::MoveSelectedPointUp()
{
  this->m_CorrespondingPointSetsModel->MoveSelectedPointUp();
}
void QmitkCorrespondingPointSetsView::ClearSelectedPointSet()
{
  switch( QMessageBox::question( this, tr("Clear point set"),
                                 tr("Remove all points from the right clicked list?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
  {
  case QMessageBox::Yes:
    {
      this->m_CorrespondingPointSetsModel->ClearSelectedPointSet();
      break;
    }
  case QMessageBox::No:
    break;
  default:
    break;
  }
}
void QmitkCorrespondingPointSetsView::ClearCurrentTimeStep()
{
  switch( QMessageBox::question( this, tr("Clear time step"),
                                 tr("Remove points from current time step of the right clicked list?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
  {
  case QMessageBox::Yes:
    {
      this->m_CorrespondingPointSetsModel->ClearCurrentTimeStep();
      break;
    }
  case QMessageBox::No:
    break;
  default:
    break;
  }
}

void QmitkCorrespondingPointSetsView::SwapPointSets(bool checked)
{
  m_swapPointSets = checked;

  if ( !checked )
  {
    std::vector<mitk::DataNode*> nodes = this->GetPointSetNodes();
    std::vector<mitk::DataNode*> reverseNodes;
    reverseNodes.push_back(nodes.back());
    reverseNodes.push_back(nodes.front());
    this->SetPointSetNodes( reverseNodes );
  }
  else
    this->SetPointSetNodes(this->GetPointSetNodes());
}
void QmitkCorrespondingPointSetsView::AddPointsMode(bool checked)
{
  m_addPointsMode = checked;
  std::vector<mitk::DataNode*> pointSetNodes = this->GetPointSetNodes();
  std::vector<mitk::DataNode*>::iterator it;
  for ( it = pointSetNodes.begin(); it < pointSetNodes.end(); it++ )
  {
    mitk::DataNode* dataNode = dynamic_cast<mitk::DataNode*>(*it);
    bool selected = false;
    dataNode->GetPropertyValue<bool>("selected", selected);

    if (selected) {
      this->UpdateSelection(dataNode);
      break;
    }
  }
}
void QmitkCorrespondingPointSetsView::UpdateSelection(mitk::DataNode* selectedNode)
{
  if (m_Interactor){
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
    m_Interactor = NULL;
  }

  if ( this->m_addPointsMode && selectedNode )
  {
    bool visible = false;
    selectedNode->GetPropertyValue<bool>("visible", visible);

    if (visible){
      // set new interactor
      m_Interactor = dynamic_cast<mitk::PointSetInteractor*>(selectedNode->GetInteractor());

      if (m_Interactor.IsNull())//if not present, instanciate one
        m_Interactor = mitk::PointSetInteractor::New("pointsetinteractor", selectedNode);
      
      //add it to global interaction to activate it
      mitk::GlobalInteraction::GetInstance()->AddInteractor( m_Interactor );
    }
  }
}