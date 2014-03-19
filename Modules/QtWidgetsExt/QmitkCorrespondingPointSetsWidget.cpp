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
#include "QmitkCorrespondingPointSetsWidget.h"
#include <mitkGlobalInteraction.h>

#include <QHBoxLayout>
#include <QStatusBar>
#include <QToolButton>

QmitkCorrespondingPointSetsWidget::QmitkCorrespondingPointSetsWidget(QWidget *parent):
  QWidget(parent),
  m_CorrespondingPointSetsView(NULL)
{
  // create new QTableView
  m_CorrespondingPointSetsView = new QmitkCorrespondingPointSetsView();
  // setup user interface
  SetupUi();
  // setup connections
  connect( this->m_CorrespondingPointSetsView, SIGNAL(SignalPointSelectionChanged()), this, SLOT(OnPointSelectionChanged()) );
  connect( this->m_CorrespondingPointSetsView, SIGNAL(SignalAddPointsModeChanged(bool)), this, SLOT(OnAddPointsModeChanged(bool)) );
}

QmitkCorrespondingPointSetsWidget::~QmitkCorrespondingPointSetsWidget()
{
  delete m_CorrespondingPointSetsView;
}

void QmitkCorrespondingPointSetsWidget::SetupUi()
{
  QBoxLayout* lay1 = new QVBoxLayout(this);

  // add status bar buttons
  if (QTPropShowButtonBar)
  {
    QBoxLayout* lay2 = new QHBoxLayout();
    lay1->addLayout(lay2);
    lay2->stretch(true);

    QStatusBar* statusBar = new QStatusBar(this);
    statusBar->setMaximumHeight(25);

    m_CreatePointSetBtn = new QToolButton();
    m_CreatePointSetBtn->setAutoRaise(true);
    m_CreatePointSetBtn->setIcon(QIcon(":/QtWidgetsExt/btnAddPointSet.png"));
    m_CreatePointSetBtn->setToolTip(QString("Create new point set"));
    connect(this->m_CreatePointSetBtn, SIGNAL(clicked()), this, SLOT(AddPointSet()));
    statusBar->addWidget(m_CreatePointSetBtn);

    m_AddPointsBtn = new QToolButton();
    m_AddPointsBtn->setAutoRaise(true);
    m_AddPointsBtn->setIcon(QIcon(":/QtWidgetsExt/btnSetPoints.png"));
    m_AddPointsBtn->setToolTip(QString("Check to add new points (shift-click)"));
    m_AddPointsBtn->setCheckable(true);
    connect(this->m_AddPointsBtn, SIGNAL(clicked(bool)), this, SLOT(AddPointsMode(bool)));
    statusBar->addWidget(m_AddPointsBtn);

    m_MovePointUpBtn = new QToolButton();
    m_MovePointUpBtn->setAutoRaise(true);
    m_MovePointUpBtn->setIcon(QIcon(":/QtWidgetsExt/btnMoveUp.png"));
    m_MovePointUpBtn->setToolTip(QString("Move selected point up"));
    connect(this->m_MovePointUpBtn, SIGNAL(clicked()), this, SLOT(MoveSelectedPointUp()));
    statusBar->addWidget(m_MovePointUpBtn);

    m_MovePointDownBtn = new QToolButton();
    m_MovePointDownBtn->setAutoRaise(true);
    m_MovePointDownBtn->setIcon(QIcon(":/QtWidgetsExt/btnMoveDown.png"));
    m_MovePointDownBtn->setToolTip(QString("Move selected point down"));
    connect(this->m_MovePointDownBtn, SIGNAL(clicked()), this, SLOT(MoveSelectedPointDown()));
    statusBar->addWidget(m_MovePointDownBtn);

    m_RemovePointBtn = new QToolButton();
    m_RemovePointBtn->setAutoRaise(true);
    m_RemovePointBtn->setIcon(QIcon(":/QtWidgetsExt/btnRemovePoint.png"));
    m_RemovePointBtn->setToolTip(QString("Remove selected point"));
    connect(this->m_RemovePointBtn, SIGNAL(clicked()), this, SLOT(RemoveSelectedPoint()));
    statusBar->addWidget(m_RemovePointBtn);

    m_SwapSetsBtn = new QToolButton();
    m_SwapSetsBtn->setAutoRaise(true);
    m_SwapSetsBtn->setIcon(QIcon(":/QtWidgetsExt/btnSwapSets.png"));
    m_SwapSetsBtn->setToolTip(QString("Swap the two selected point sets"));
    m_SwapSetsBtn->setCheckable(true);
    connect(this->m_SwapSetsBtn, SIGNAL(clicked(bool)), this, SLOT(SwapPointSets(bool)));
    statusBar->addWidget(m_SwapSetsBtn);

    // disable buttons
    m_MovePointUpBtn->setEnabled(false);
    m_MovePointDownBtn->setEnabled(false);
    m_RemovePointBtn->setEnabled(false);
    m_SwapSetsBtn->setEnabled(false);
    m_AddPointsBtn->setEnabled(false);

    lay2->addWidget(statusBar);
  }

  lay1->insertWidget(0,m_CorrespondingPointSetsView);
  this->setLayout(lay1);
}

void QmitkCorrespondingPointSetsWidget::SetPointSetNodes(std::vector<mitk::DataNode*> nodes)
{
  this->m_CorrespondingPointSetsView->SetPointSetNodes(nodes);
  if (this->GetPointSetNodes().size()<2)
    m_SwapSetsBtn->setEnabled(false);
  else
    m_SwapSetsBtn->setEnabled(true);
}

std::vector<mitk::DataNode*> QmitkCorrespondingPointSetsWidget::GetPointSetNodes()
{
  return this->m_CorrespondingPointSetsView->GetPointSetNodes();
}

void QmitkCorrespondingPointSetsWidget::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
    m_CorrespondingPointSetsView->SetMultiWidget(multiWidget);
}

void QmitkCorrespondingPointSetsWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
    m_CorrespondingPointSetsView->SetDataStorage(dataStorage);
}

void QmitkCorrespondingPointSetsWidget::UpdateSelection(mitk::DataNode* selectedNode)
{
  m_AddPointsBtn->setEnabled(false);
  if (!selectedNode || !(dynamic_cast<mitk::PointSet*>(selectedNode->GetData())))
    return;

  bool visible = false;
  selectedNode->GetPropertyValue<bool>("visible", visible);

  if (visible){
    m_AddPointsBtn->setEnabled(true);
    m_CorrespondingPointSetsView->UpdateSelection(selectedNode);
  }
}

bool QmitkCorrespondingPointSetsWidget::QTPropButtonBarEnabled() const
{
  return this->QTPropShowButtonBar;
}

void QmitkCorrespondingPointSetsWidget::QTPropSetButtonBarEnabled(bool showBB)
{
  this->QTPropShowButtonBar = showBB;
}

void QmitkCorrespondingPointSetsWidget::AddPointSet()
{
  m_CorrespondingPointSetsView->AddPointSet();
}

void QmitkCorrespondingPointSetsWidget::OnPointSelectionChanged()
{
  if ( this->m_CorrespondingPointSetsView->IsPointSelected() )
  {
    m_MovePointUpBtn->setEnabled(true);
    m_MovePointDownBtn->setEnabled(true);
    m_RemovePointBtn->setEnabled(true);
  }
  else
  {
    m_MovePointUpBtn->setEnabled(false);
    m_MovePointDownBtn->setEnabled(false);
    m_RemovePointBtn->setEnabled(false);
  }
}

void QmitkCorrespondingPointSetsWidget::RemoveSelectedPoint()
{
  this->m_CorrespondingPointSetsView->RemoveSelectedPoint();
}

void QmitkCorrespondingPointSetsWidget::MoveSelectedPointDown()
{
  this->m_CorrespondingPointSetsView->MoveSelectedPointDown();
}

void QmitkCorrespondingPointSetsWidget::MoveSelectedPointUp()
{
  this->m_CorrespondingPointSetsView->MoveSelectedPointUp();
}

void QmitkCorrespondingPointSetsWidget::AddPointsMode(bool checked)
{
  this->m_CorrespondingPointSetsView->AddPointsMode(checked);
}

void QmitkCorrespondingPointSetsWidget::OnAddPointsModeChanged(bool enabled)
{
  this->m_AddPointsBtn->setChecked(enabled);
}

void QmitkCorrespondingPointSetsWidget::SwapPointSets(bool checked)
{
  this->m_CorrespondingPointSetsView->SwapPointSets(checked);
}

