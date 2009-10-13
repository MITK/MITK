/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15412 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>
#include <cherryPlatform.h>

#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkIDataStorageService.h>

#include "QmitkMeasurement.h"

#include <QGridLayout>
#include <QMainWindow>
#include <QToolBar>
#include <QTableWidget>


QmitkMeasurement::QmitkMeasurement()
: m_PointSetInteractor(0)
, m_CurrentPointSetNode(0)
, m_MainWindow(0)
, m_Layout(0)
, m_DrawActionsToolBar(0)
, m_DrawItemsTableWidget(0)
{
}

QmitkMeasurement::~QmitkMeasurement()
{
  this->Deactivated();

  if(m_MainWindow && m_DrawActionsToolBar)
  {
    m_MainWindow->removeToolBar(m_DrawActionsToolBar);
  }
}

void QmitkMeasurement::CreateQtPartControl( QWidget* parent )
{
  m_DrawActionsToolBar = new QToolBar;
  m_DrawActionsMainWindowToolBar = new QToolBar;

  //# add actions
  QAction* currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/line.png"), "Draw Line");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawLineTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/path.png"), "Draw Path");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawPathTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/angle.png"), "Draw Angle");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawAngleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/four-point-angle.png"), "Draw Four Point Angle");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawFourPointAngleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/ellipse.png"), "Draw Ellipse");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawEllipseTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/rectangle.png"), "Draw Rectangle");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawRectangleTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/polygon.png"), "Draw Polygon");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawPolygonTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/arrow.png"), "Draw Arrow");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawArrowTriggered(bool) ) );

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/text.png"), "Draw Text");
  m_DrawActionsMainWindowToolBar->addAction(currentAction);
  QObject::connect( currentAction, SIGNAL( triggered(bool) )
    , this, SLOT( ActionDrawTextTriggered(bool) ) );

  m_DrawItemsTableWidget = new QTableWidget;

  m_Layout = new QGridLayout;
  //m_Layout->addWidget(m_DrawActionsToolBar, 0, 0, 1, 1);
  m_Layout->addWidget(m_DrawItemsTableWidget, 0, 0, 1, 1);

  parent->setLayout(m_Layout);

  m_MainWindow = qobject_cast<QMainWindow*> ( QApplication::activeWindow() );

  if(m_MainWindow)
  {
    m_MainWindow->addToolBar(Qt::LeftToolBarArea, m_DrawActionsMainWindowToolBar);
    m_DrawActionsMainWindowToolBar->setFloatable(false);
    m_DrawActionsMainWindowToolBar->setMovable(false);
  }
}

void QmitkMeasurement::Activated()
{
  m_PointSetInteractor = 0;
  m_CurrentPointSetNode = 0;

  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
  this->GetActiveStdMultiWidget()->changeLayoutToWidget1();

  if(m_MainWindow)
  {
    m_DrawActionsMainWindowToolBar->setVisible(true);
  }
}

void QmitkMeasurement::Deactivated()
{
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

  if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
  {
    this->GetDataStorage()->Remove(m_CurrentPointSetNode);
  }

  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(true);
  this->GetActiveStdMultiWidget()->changeLayoutToDefault();

  if(m_MainWindow)
  {
    m_DrawActionsMainWindowToolBar->setVisible(false);
  }
}


void QmitkMeasurement::Reset()
{
  //make sure the last interactor that has been used is removed
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

  // if the last point set was not used for a measurement delete it
  if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
  {
    this->GetDataStorage()->Remove(m_CurrentPointSetNode);
  }
}

void QmitkMeasurement::ActionDrawLineTriggered( bool checked )
{
  this->Reset();

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("distance"));
  m_CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));

  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode, 2);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::ActionDrawPathTriggered( bool checked )
{
  this->Reset();

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("path"));
  m_CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  //m_DataTreeIterator->Add(m_CurrentPointSetNode);
  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::ActionDrawAngleTriggered( bool checked )
{
  this->Reset();

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("angle"));
  m_CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode, 3);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::ActionDrawFourPointAngleTriggered( bool checked )
{

}

void QmitkMeasurement::ActionDrawEllipseTriggered( bool checked )
{

}

void QmitkMeasurement::ActionDrawRectangleTriggered( bool checked )
{

}

void QmitkMeasurement::ActionDrawPolygonTriggered( bool checked )
{
  this->Reset();

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("path"));
  m_CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  //m_DataTreeIterator->Add(m_CurrentPointSetNode);
  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::ActionDrawArrowTriggered( bool checked )
{

}

void QmitkMeasurement::ActionDrawTextTriggered( bool checked )
{

}