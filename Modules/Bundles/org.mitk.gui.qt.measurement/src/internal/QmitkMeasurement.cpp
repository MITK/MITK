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

#include "mitkGlobalInteraction.h"
#include "mitkPointSet.h"
#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkIDataStorageService.h"
#include "mitkDataTreeNodeObject.h"
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateAND.h>

#include "mitkPlanarCircle.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarAngle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarFigureInteractor.h"

#include "QmitkMeasurement.h"

#include <QGridLayout>
#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QTableView>
#include <QmitkDataStorageTableModel.h>
#include "mitkNodePredicateDataType.h"
#include "mitkPlanarFigure.h"

QmitkMeasurement::QmitkMeasurement()
: m_PointSetInteractor(0)
, m_CurrentPointSetNode(0)
, m_MainWindow(0)
, m_Layout(0)
, m_DrawActionsToolBar(0)
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

  currentAction = m_DrawActionsToolBar->addAction(QIcon(":/measurement/circle.png"), "Draw Ellipse");
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

  //# m_NodeTableModel
  mitk::NodePredicateProperty::Pointer isHelperObject = mitk::NodePredicateProperty::New("helper object"
    , mitk::BoolProperty::New(true));

  mitk::NodePredicateNOT::Pointer isNotHelperObject 
    = mitk::NodePredicateNOT::New(isHelperObject);// Show only nodes that really contain dat

  mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

  mitk::NodePredicateAND::Pointer dataIsPlanarFigureAndIsNotHelperObject = mitk::NodePredicateAND::New(isPlanarFigure,
    isNotHelperObject);

  m_PlanarFiguresModel = new QmitkDataStorageTableModel(this->GetDefaultDataStorage(), dataIsPlanarFigureAndIsNotHelperObject);

  QLabel* selectedImageLabel = new QLabel("Selected Image: ");
  m_SelectedImage = new QLabel("None selected!");

  //# m_PlanarFiguresTable
  m_PlanarFiguresTable = new QTableView;
  //m_PlanarFiguresTable->setContextMenuPolicy(Qt::CustomContextMenu);
  m_PlanarFiguresTable->setSelectionMode( QAbstractItemView::ExtendedSelection );
  m_PlanarFiguresTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_PlanarFiguresTable->horizontalHeader()->setStretchLastSection(true);
  m_PlanarFiguresTable->setAlternatingRowColors(true);
  m_PlanarFiguresTable->setSortingEnabled(true);
  m_PlanarFiguresTable->setModel(m_PlanarFiguresModel);

  m_Layout = new QGridLayout;
  //m_Layout->addWidget(m_DrawActionsToolBar, 0, 0, 1, 1);
  m_Layout->addWidget(selectedImageLabel, 0, 0, 1, 1);
  m_Layout->addWidget(m_SelectedImage, 0, 1, 1, 1);
  m_Layout->addWidget(m_PlanarFiguresTable, 1, 0, 1, 2);
  m_Layout->setRowStretch(0, 1);
  m_Layout->setRowStretch(1, 10);

  parent->setLayout(m_Layout);

  m_MainWindow = qobject_cast<QMainWindow*> ( QApplication::activeWindow() );

  if(m_MainWindow)
  {
    m_MainWindow->addToolBar(Qt::LeftToolBarArea, m_DrawActionsMainWindowToolBar);
    m_DrawActionsMainWindowToolBar->setFloatable(false);
    m_DrawActionsMainWindowToolBar->setMovable(false);
  }

  // Initialize selection listener mechanism
  m_SelectionListener = cherry::ISelectionListener::Pointer(
    new cherry::SelectionChangedAdapter< QmitkMeasurement >(
      this, &QmitkMeasurement::SelectionChanged) );
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
  cherry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  m_CurrentSelection = selection.Cast< const cherry::IStructuredSelection >();
  this->SelectionChanged(cherry::SmartPointer<IWorkbenchPart>(NULL), selection);

}

void QmitkMeasurement::Activated()
{
  m_PointSetInteractor = 0;
  m_CurrentPointSetNode = 0;
}

void QmitkMeasurement::Deactivated()
{
//   if (m_PointSetInteractor.IsNotNull())
//   {
//     mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
//   }
// 
//   if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
//   {
//     this->GetDataStorage()->Remove(m_CurrentPointSetNode);
//   }
}


void QmitkMeasurement::SelectionChanged( cherry::IWorkbenchPart::Pointer sourcepart, 
  cherry::ISelection::ConstPointer selection )
{
  LOG_INFO << "SelectionChanged.";
  // By default: no image selected

  if ( sourcepart == this ||  // prevents being notified by own selection events
    !selection.Cast<const cherry::IStructuredSelection>() ) // checks that the selection is a IStructuredSelection and not NULL
  {
    LOG_INFO << "Selection failed.";
    return; // otherwise we get "null selection" events each time the view is activated/focussed
  }

  // save current selection in member variable
  m_CurrentSelection = selection.Cast< const cherry::IStructuredSelection >();

  if ( m_CurrentSelection.IsNull() || (m_CurrentSelection->Size() != 1) )
  {
    LOG_INFO << "Selection invalid.";
    return;
  }

  // Get selected element
  //mitk::DataTreeNodeObject *nodeObject = 
  //  m_CurrentSelection->GetFirstElement().Cast< mitk::DataTreeNodeObject >();
  cherry::IStructuredSelection::iterator it = m_CurrentSelection->Begin();
  mitk::DataTreeNodeObject::Pointer nodeObject = it->Cast <mitk::DataTreeNodeObject >();

  mitk::DataTreeNode *node = nodeObject->GetDataTreeNode();

  // Check if an image has been selected
  if ( node->GetData() && dynamic_cast< mitk::Image * >( node->GetData() ) )
  {
    // YES: node becomes new selected image node
    m_SelectedImageNode = node;
    m_SelectedImage->setText(QString::fromStdString(m_SelectedImageNode->GetName()));
  }
  else
  {
    m_SelectedImageNode = NULL;
    m_SelectedImage->setText("None selected!");
  }
}


void QmitkMeasurement::ActionDrawLineTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }

  mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
    this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry() );

  mitk::PlanarLine::Pointer figure = mitk::PlanarLine::New();
  figure->SetGeometry2D( planeGeometry );

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarLine initialized...";

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawPathTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }

  mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
    this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry() );

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOff();
  figure->SetGeometry2D( planeGeometry );

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarPolygon initialized...";

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawAngleTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }

  mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
    this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry() );

  mitk::PlanarAngle::Pointer figure = mitk::PlanarAngle::New();
  figure->SetGeometry2D( planeGeometry );

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarAngle initialized...";

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawFourPointAngleTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }
}

void QmitkMeasurement::ActionDrawEllipseTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }

  mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
    this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry() );

  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
  figure->SetGeometry2D( planeGeometry );

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarCircle initialized...";

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}

void QmitkMeasurement::ActionDrawRectangleTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }
}

void QmitkMeasurement::ActionDrawPolygonTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }

  mitk::PlaneGeometry *planeGeometry = const_cast< mitk::PlaneGeometry * >(
    this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry() );

  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();
  figure->SetGeometry2D( planeGeometry );

  mitk::DataTreeNode::Pointer figureNode = mitk::DataTreeNode::New();
  figureNode->SetData( figure );

  this->GetDataStorage()->Add( figureNode, m_SelectedImageNode );

  mitk::PlanarFigureInteractor::Pointer interactor  =
    mitk::PlanarFigureInteractor::New( "PlanarFigureInteractor", figureNode );
  mitk::GlobalInteraction::GetInstance()->AddInteractor( interactor );

  LOG_INFO << "PlanarPolygon initialized...";

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
}


void QmitkMeasurement::ActionDrawArrowTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }
}

void QmitkMeasurement::ActionDrawTextTriggered( bool  /*checked*/ )
{
  if ( m_SelectedImageNode.IsNull() )
  {
    return;
  }
}

void QmitkMeasurement::Visible()
{


  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
  this->GetActiveStdMultiWidget()->changeLayoutToWidget1();

  if(m_MainWindow)
  {
    m_DrawActionsMainWindowToolBar->setVisible(true);
  }
}

void QmitkMeasurement::Hidden()
{

  this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(true);
  this->GetActiveStdMultiWidget()->changeLayoutToDefault();

  if(m_MainWindow)
  {
    m_DrawActionsMainWindowToolBar->setVisible(false);
  }

}