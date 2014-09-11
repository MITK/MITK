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
#include <QInputDialog>
#include <QMetaProperty>
#include <mitkStepper.h>
#include <QTableWidget>

QmitkCorrespondingPointSetsView::QmitkCorrespondingPointSetsView( QWidget* parent )
  :QTableView( parent ),
  m_CorrespondingPointSetsModel( new QmitkCorrespondingPointSetsModel() ),
  m_SelfCall( false ),
  m_swapPointSets(false),
  m_addPointsMode(false),
  m_DataStorage( NULL )
{
  m_CorrespondingPointSetsModel->setProperty("QTPropShowCoordinates", true);
  m_CorrespondingPointSetsModel->setProperty("QTPropShowIds", true);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  this->setToolTip("Use right click to open context menu");

  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setDropIndicatorShown(true);
  this->setDragDropMode(QAbstractItemView::InternalMove);

  QTableView::setAlternatingRowColors( false );
  QTableView::setSelectionBehavior( QAbstractItemView::SelectItems );
  QTableView::setSelectionMode( QAbstractItemView::SingleSelection );
  QTableView::setModel( m_CorrespondingPointSetsModel );
  QTableView::horizontalHeader()->resizeSection(0, (int)(this->width()/3.5));
  QTableView::horizontalHeader()->setStretchLastSection(true);

  m_TimeStepFaderLabel = new QLabel(this);
  QFont font("Arial", 17);
  m_TimeStepFaderLabel->setFont(font);

  connect( QTableView::selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
           this, SLOT(OnPointSelectionChanged(const QItemSelection& , const QItemSelection&)) );

  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenu(const QPoint &)));

  connect(this->m_CorrespondingPointSetsModel, SIGNAL(SignalPointSetChanged()), this, SLOT(UpdateSelectionHighlighting()));
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
  this->m_CorrespondingPointSetsModel->SetMultiWidget(multiWidget);
}

QmitkStdMultiWidget* QmitkCorrespondingPointSetsView::GetMultiWidget() const
{
  return this->m_CorrespondingPointSetsModel->GetMultiWidget();
}

void QmitkCorrespondingPointSetsView::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCorrespondingPointSetsView::OnPointSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
  if(m_SelfCall)
    return;

  std::vector<mitk::DataNode*> pointSetNodes = this->GetPointSetNodes();

  QModelIndexList selectedIndexes = selected.indexes();
  m_CorrespondingPointSetsModel->SetSelectedPointSetIndex(-1);
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
      this->m_CorrespondingPointSetsModel->UpdateSelection(pointSetNode);
      pointSet = dynamic_cast<mitk::PointSet*>(pointSetNode->GetData());

      if( pointSet->GetPointSet(m_CorrespondingPointSetsModel->GetTimeStep()))
      for (mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_CorrespondingPointSetsModel->GetTimeStep())->GetPoints()->Begin();
        it != pointSet->GetPointSet(m_CorrespondingPointSetsModel->GetTimeStep())->GetPoints()->End(); ++it)
        {
          QModelIndex tempIndex;
          if (m_CorrespondingPointSetsModel->GetModelIndexForPointID(it->Index(), tempIndex, index.column()))
          {
            if (tempIndex == index)
            {
              pointSet->SetSelectInfo(it->Index(), true, m_CorrespondingPointSetsModel->GetTimeStep());

              m_CorrespondingPointSetsModel->SetSelectedPointSetIndex(index.column());
              if ( this->GetMultiWidget() != NULL)
              {
                this->GetMultiWidget()->MoveCrossToPosition(pointSet->GetPoint(it->Index(), m_CorrespondingPointSetsModel->GetTimeStep()));
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
  emit(SignalPointSelectionChanged());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->UpdateSelectionHighlighting();
  m_SelfCall = false;
}


void QmitkCorrespondingPointSetsView::keyPressEvent( QKeyEvent * e )
{
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
  if (!this->m_CorrespondingPointSetsModel->GetStepper())
    return;

  int whe = event->delta();
  int pos = this->m_CorrespondingPointSetsModel->GetStepper()->GetPos();

  int currentTS =  this->m_CorrespondingPointSetsModel->GetTimeStep();
  if(whe > 0)
  {
    this->m_CorrespondingPointSetsModel->SetTimeStep(++currentTS);
    this->m_CorrespondingPointSetsModel->GetStepper()->SetPos(++pos);
  }
  else if( pos > 0 )
  {
    this->m_CorrespondingPointSetsModel->SetTimeStep(--currentTS);
    this->m_CorrespondingPointSetsModel->GetStepper()->SetPos(--pos);
  }
  fadeTimeStepIn();
  emit SignalPointSelectionChanged();
}

void QmitkCorrespondingPointSetsView::fadeTimeStepIn()
{
  if (!this->m_CorrespondingPointSetsModel->GetStepper())
    return;

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
  m_TimeStepFaderLabel->setText(QString("%1").arg(this->m_CorrespondingPointSetsModel->GetStepper()->GetPos()));

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

  int numNodes = this->GetPointSetNodes().size();

  //add delete point action
  int id;
  mitk::PointSet::PointType p;
  bool pointSelected = m_CorrespondingPointSetsModel->GetPointForModelIndex(row, col, p, id);

  QAction *movePointUp = new QAction(this);
  movePointUp->setText("Move point up");
  connect(movePointUp, SIGNAL(triggered()), this, SLOT(MoveSelectedPointUp()));
  if(!pointSelected)
    movePointUp->setEnabled(false);
  menu->addAction(movePointUp);

  QAction *movePointDown = new QAction(this);
  movePointDown->setText("Move point down");
  connect(movePointDown, SIGNAL(triggered()), this, SLOT(MoveSelectedPointDown()));
  if(!pointSelected)
    movePointDown->setEnabled(false);
  menu->addAction(movePointDown);

  QAction *delPoint = new QAction(this);
  delPoint->setText("Delete point");
  connect(delPoint, SIGNAL(triggered()), this, SLOT(RemoveSelectedPoint()));
  if(!pointSelected)
    delPoint->setEnabled(false);
  menu->addAction(delPoint);

  QAction *separator = new QAction(this);
  separator->setSeparator(true);

  menu->addSeparator();

  QAction *clearTS = new QAction(this);
  clearTS->setText("Clear time step");
  connect(clearTS, SIGNAL(triggered()), this, SLOT(ClearCurrentTimeStep()));
  if(numNodes==0 || (col!=0 && col!=1))
    clearTS->setEnabled(false);
  menu->addAction(clearTS);

  QAction *clearList = new QAction(this);
  clearList->setText("Clear point set");
  connect(clearList, SIGNAL(triggered()), this, SLOT(ClearSelectedPointSet()));
  if(numNodes==0 || (col!=0 && col!=1))
    clearList->setEnabled(false);
  menu->addAction(clearList);

  menu->addSeparator();

  QAction *swapSets = new QAction(this);
  swapSets->setText("Swap point sets");
  connect(swapSets, SIGNAL(triggered(bool)), this, SLOT(SwapPointSets(bool)));
  swapSets->setCheckable(true);
  swapSets->setChecked(m_swapPointSets);
  if (numNodes<2)
    swapSets->setEnabled(false);
  menu->addAction(swapSets);

  QAction *addPoints = new QAction(this);
  addPoints->setText("Check to add new points");
  connect(addPoints, SIGNAL(triggered(bool)), this, SLOT(AddPointsMode(bool)));
  addPoints->setCheckable(true);
  addPoints->setChecked(m_addPointsMode);
  if (numNodes==0)
    addPoints->setEnabled(false);
  menu->addAction(addPoints);

  QAction *addPointSet = new QAction(this);
  addPointSet->setText("Create new point set");
  connect(addPointSet, SIGNAL(triggered()), this, SLOT(AddPointSet()));
  if (!m_DataStorage)
    addPointSet->setEnabled(false);
  menu->addAction(addPointSet);

  menu->exec(this->mapToGlobal(pos));
}

std::vector<mitk::DataNode*> QmitkCorrespondingPointSetsView::GetPointSetNodes()
{
  return this->m_CorrespondingPointSetsModel->GetPointSetNodes();
}

std::vector<mitk::PointSet*> QmitkCorrespondingPointSetsView::GetPointSets()
{
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

void QmitkCorrespondingPointSetsView::RemoveSelectedPoint()
{
  this->m_CorrespondingPointSetsModel->RemoveSelectedPoint();
  emit(SignalPointSelectionChanged());
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
  emit(SignalPointSelectionChanged());
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
  emit(SignalPointSelectionChanged());
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

  m_CorrespondingPointSetsModel->SetSelectedPointSetIndex((m_CorrespondingPointSetsModel->GetSelectedPointSetIndex()+1)%2);
  this->UpdateSelectionHighlighting();
}

void QmitkCorrespondingPointSetsView::AddPointsMode(bool checked)
{
  m_addPointsMode = checked;
  std::vector<mitk::DataNode*> pointSetNodes = this->GetPointSetNodes();
  std::vector<mitk::DataNode*>::iterator it;

  bool selected = false;
  if (checked)
  {
    for ( it = pointSetNodes.begin(); it < pointSetNodes.end(); it++ )
    {
      mitk::DataNode* dataNode = dynamic_cast<mitk::DataNode*>(*it);
      dataNode->GetPropertyValue<bool>("selected", selected);
      if (selected) {
        this->UpdateSelection(dataNode);
        break;
      }
    }
  }
  else
  {
    this->m_CorrespondingPointSetsModel->RemoveInteractor();
  }

  m_addPointsMode = selected;
  emit SignalAddPointsModeChanged(selected);
}

void QmitkCorrespondingPointSetsView::UpdateSelection(mitk::DataNode* selectedNode)
{
  this->m_CorrespondingPointSetsModel->UpdateSelection(selectedNode);
}

void QmitkCorrespondingPointSetsView::AddPointSet()
{
  //Ask for the name of the point set
  bool ok = false;
  QString name = QInputDialog::getText( QApplication::activeWindow()
    , "Add point set...", "Enter name for the new point set", QLineEdit::Normal, "PointSet", &ok );
  if ( ! ok || name.isEmpty() )
    return;

  //
  //Create a new empty pointset
  //
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  //
  // Create a new data tree node
  //
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  //
  // fill the data tree node with the appropriate information
  //
  pointSetNode->SetData( pointSet );
  pointSetNode->SetProperty( "name", mitk::StringProperty::New( name.toStdString() ) );
  pointSetNode->SetProperty( "opacity", mitk::FloatProperty::New( 1 ) );
  pointSetNode->SetColor( 1.0, 1.0, 0.0 );
  //
  // add the node to the ds
  //
  this->m_DataStorage->Add(pointSetNode);
}

bool QmitkCorrespondingPointSetsView::IsPointSelected()
{
  if ( this->m_CorrespondingPointSetsModel->GetSelectedPointSetIndex()>=0 )
    return true;
  return false;
}

QmitkCorrespondingPointSetsModel* QmitkCorrespondingPointSetsView::GetModel()
{
  return this->m_CorrespondingPointSetsModel;
}

void QmitkCorrespondingPointSetsView::UpdateSelectionHighlighting()
{
  this->m_SelfCall = true;
  QModelIndex index;
  bool modelIndexOkay = this->m_CorrespondingPointSetsModel->GetModelIndexForSelectedPoint(index);
  if(!modelIndexOkay){
    this->m_SelfCall = false;
    return;
  }
  QTableView::selectionModel()->select( index , QItemSelectionModel::ClearAndSelect );
  this->setFocus();
  this->m_SelfCall = false;
}

