/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkGeometry2DDataMapper2D.h"
#include "mitkOpenGLRenderer.h"
#include "mitkPositionEvent.h"
#include "mitkInteractionConst.h"
#include "mitkGlobalInteraction.h"
#include "mitkPositionTracker.h"
#include "mitkPointSet.h"
#include "mitkStateEvent.h"
#include "mitkPlaneGeometry.h"
#include "mitkLine.h"

#include <qapplication.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>


void QmitkStdMultiWidget::init()
{ 
  mitkWidget1->setPaletteBackgroundColor("red");
  mitkWidget2->setPaletteBackgroundColor("green");
  mitkWidget3->setPaletteBackgroundColor("blue");
  mitkWidget4->setPaletteBackgroundColor("yellow");

  planesIterator = NULL;
  m_PositionTracker = NULL;

  // transfer colors in WorldGeometry-Nodes of the associated Renderer
  QColor qcolor;
  float color[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  mitk::IntProperty::Pointer  layer;

  // ... of widget 1
  qcolor=mitkWidget1->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0;
  color[1]=qcolor.green()/255.0;
  color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);

  // ... of widget 2
  qcolor=mitkWidget2->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0;
  color[1]=qcolor.green()/255.0;
  color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);

  // ... of widget 3
  qcolor=mitkWidget3->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0;
  color[1]=qcolor.green()/255.0;
  color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);

  // ... of widget 4
  qcolor=mitkWidget4->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0;
  color[1]=qcolor.green()/255.0; 
  color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget4->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);

  // create a slice rotator
  // m_SlicesRotator = mitk::SlicesRotator::New();
  // @TODO next line causes sure memory leak
  // rotator will be created nonetheless (will be switched on and off)
  m_SlicesRotation = false; 
  m_SlicesRotator = new mitk::SlicesRotator("slices-rotator");
  m_SlicesRotator->AddSliceController( 
    mitkWidget1->GetSliceNavigationController() );
  m_SlicesRotator->AddSliceController( 
    mitkWidget2->GetSliceNavigationController() );
  m_SlicesRotator->AddSliceController(
    mitkWidget3->GetSliceNavigationController() );

  //initialize timeNavigationController: send time via sliceNavigationControllers
  timeNavigationController = mitk::SliceNavigationController::New(NULL);
  timeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget1->GetSliceNavigationController(), false);
  timeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget2->GetSliceNavigationController(), false);
  timeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget3->GetSliceNavigationController(), false);
  timeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget4->GetSliceNavigationController(), false);
  mitkWidget1->GetSliceNavigationController()
    ->ConnectGeometrySendEvent(mitkWidget4->GetRenderer());

  //reverse connection between sliceNavigationControllers and timeNavigationController
  mitkWidget1->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);
  mitkWidget2->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);
  mitkWidget3->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);
  mitkWidget4->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);

  // instantiate display interactor
  m_MoveAndZoomInteractor = mitk::DisplayVectorInteractor::New(
    "moveNzoom", new mitk::DisplayInteractor() );
  
  m_LastLeftClickPositionSupplier = 
    mitk::CoordinateSupplier::New("navigation", NULL);
  mitk::GlobalInteraction::GetInstance()->AddListener(
    m_LastLeftClickPositionSupplier
  );
  // setup gradient background
  m_GradientBackground1 = mitk::GradientBackground::New();
  m_GradientBackground1->SetRenderWindow(
    mitkWidget1->GetRenderer()->GetRenderWindow() );
  m_GradientBackground1->Disable();
  
  m_GradientBackground2 = mitk::GradientBackground::New();
  m_GradientBackground2->SetRenderWindow( 
    mitkWidget2->GetRenderer()->GetRenderWindow() );
  m_GradientBackground2->Disable();
  
  m_GradientBackground3 = mitk::GradientBackground::New();
  m_GradientBackground3->SetRenderWindow( 
    mitkWidget3->GetRenderer()->GetRenderWindow() );
  m_GradientBackground3->Disable();
  
  m_GradientBackground4 = mitk::GradientBackground::New();
  m_GradientBackground4->SetRenderWindow(
    mitkWidget4->GetRenderer()->GetRenderWindow() );
  m_GradientBackground4->SetGradientColors(0.0,0.1,0.3,0.7,0.7,0.8);
  m_GradientBackground4->Enable();
  // setup the department logo rendering
  m_LogoRendering1 = mitk::LogoRendering::New();
  m_LogoRendering1->SetRenderWindow(
    mitkWidget1->GetRenderer()->GetRenderWindow() );
  m_LogoRendering1->Disable();

  m_LogoRendering2 = mitk::LogoRendering::New();
  m_LogoRendering2->SetRenderWindow(
    mitkWidget2->GetRenderer()->GetRenderWindow() );
  m_LogoRendering2->Disable();

  m_LogoRendering3 = mitk::LogoRendering::New();
  m_LogoRendering3->SetRenderWindow(
    mitkWidget3->GetRenderer()->GetRenderWindow() );
  m_LogoRendering3->Disable();
  
  m_LogoRendering4 = mitk::LogoRendering::New();
  m_LogoRendering4->SetRenderWindow(
    mitkWidget4->GetRenderer()->GetRenderWindow() );
  m_LogoRendering4->Enable();
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesUp()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2D images up... " << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0,02, "QmitkStdMultiWidgetLayout");

  QHBoxLayout * layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  QGridLayout* layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");

  QHBoxLayout *TwoDLayout = new QHBoxLayout( 0, 0, 6, "2DLayout");

  mitkWidget1->setMaximumSize(2000,200);
  mitkWidget2->setMaximumSize(2000,200);
  mitkWidget3->setMaximumSize(2000,200);

  TwoDLayout->addWidget( mitkWidget1 );
  TwoDLayout->addWidget( mitkWidget2 );
  TwoDLayout->addWidget( mitkWidget3 );

  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout3->addLayout( TwoDLayout , 0, 0 );
  layout3->addWidget( mitkWidget4, 1, 0 );

  layout4->addLayout( layout3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_2D_IMAGES_UP;
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesLeft()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2D images left... " << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0,0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  QGridLayout *layout3 = new QGridLayout( 0, 1, 2, 0, 6, "layout3");

  QVBoxLayout *TwoDLayout = new QVBoxLayout( 0, 0, 6, "2DLayout");

  mitkWidget1->setMaximumSize(300,300);
  mitkWidget2->setMaximumSize(300,300);
  mitkWidget3->setMaximumSize(300,300);

  TwoDLayout->addWidget( mitkWidget1 );
  TwoDLayout->addWidget( mitkWidget2 );
  TwoDLayout->addWidget( mitkWidget3 );

  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout3->addLayout( TwoDLayout , 0, 0 );
  layout3->addWidget( mitkWidget4, 0,1 );

  layout4->addLayout( layout3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_2D_IMAGES_LEFT;
}

void QmitkStdMultiWidget::changeLayoutToDefault()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to default... " << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  QGridLayout *layout3 = new QGridLayout( 0, 2, 2, 0, 6, "layout3");

  mitkWidget1->setMaximumSize(2000,2000);
  mitkWidget2->setMaximumSize(2000,2000);
  mitkWidget3->setMaximumSize(2000,2000);

  layout3->addWidget( mitkWidget1,0,0 );
  layout3->addWidget( mitkWidget2,0,1 );
  layout3->addWidget( mitkWidget3,1,0 );
  layout3->addWidget( mitkWidget4,1,1 );

  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout4->addLayout( layout3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_DEFAULT;
}

void QmitkStdMultiWidget::changeLayoutToBig3D()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big 3D ..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  mitkWidget1->hide();
  mitkWidget2->hide();
  mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout4->addWidget( mitkWidget4 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_BIG_3D;
}

void QmitkStdMultiWidget::changeLayoutToWidget1()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big Widget1 ..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->setMaximumSize(2000,2000);

  mitkWidget2->hide();
  mitkWidget3->hide();
  mitkWidget4->hide();
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();

  layout4->addWidget( mitkWidget1 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_WIDGET1;
}

void QmitkStdMultiWidget::changeLayoutToWidget2()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big Widget2 ..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget2->setMaximumSize(2000,2000);

  mitkWidget1->hide();
  mitkWidget3->hide();
  mitkWidget4->hide();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();

  layout4->addWidget( mitkWidget2 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_WIDGET2;
}

void QmitkStdMultiWidget::changeLayoutToWidget3()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big Widget1 ..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget3->setMaximumSize(2000,2000);

  mitkWidget1->hide();
  mitkWidget2->hide();
  mitkWidget4->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();

  layout4->addWidget( mitkWidget3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_WIDGET3;
}

void QmitkStdMultiWidget::changeLayoutToRowWidget3And4()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to Widget3 and 4 in a Row..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QGridLayout *layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  mitkWidget3->setMaximumSize(2000,1000);
  mitkWidget4->setMaximumSize(2000,1000);

  layout3->addWidget( mitkWidget3,0,0);
  layout3->addWidget( mitkWidget4,1,0);

  layout4->addLayout(layout3);
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_ROW_WIDGET_3_AND_4;
}

void QmitkStdMultiWidget::changeLayoutToColumnWidget3And4()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to Widget3 and 4 in one Column..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  mitkWidget3->setMaximumSize(1000,2000);
  mitkWidget4->setMaximumSize(1000,2000);

  layout4->addWidget( mitkWidget3);
  layout4->addWidget( mitkWidget4);

  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_COLUMN_WIDGET_3_AND_4;
}

void QmitkStdMultiWidget::changeLayoutToRowWidgetSmall3andBig4()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to Widget3 and 4 in a Row..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QGridLayout *layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  mitkWidget3->setMaximumSize(2000,200);
  mitkWidget4->setMaximumSize(2000,1800);

  layout3->addWidget( mitkWidget3,0,0);
  layout3->addWidget( mitkWidget4,1,0);

  layout4->addLayout(layout3);
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_ROW_WIDGET_SMALL3_AND_BIG4;
}


void QmitkStdMultiWidget::changeLayoutToSmallUpperWidget2Big3and4()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to Widget3 and 4 in a Row..." << std::endl;
  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QGridLayout *layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  QGridLayout *layout5 = new QGridLayout( 0, 1, 2, 0, 6, "layout5");
  

  mitkWidget1->hide();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  mitkWidget2->setMaximumSize(2000,200);
  mitkWidget3->setMaximumSize(1000,1800);
  mitkWidget4->setMaximumSize(1000,1800);

  layout5->addWidget(mitkWidget3,0,0);
  layout5->addWidget(mitkWidget4,0,1);
  
  layout3->addWidget( mitkWidget2,0,0);
  layout3->addLayout(layout5,1,0);

  layout4->addLayout(layout3);
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

  m_Layout = LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4;
}

void QmitkStdMultiWidget::changeLayoutTo2x2Dand3DWidget()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2 x 2D and 3D Widget" << std::endl;

  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QGridLayout *layout1 = new QGridLayout( 0, 2, 1, 0, 6, "layout1" );
  QHBoxLayout *layout2 = new QHBoxLayout( 0, 0, 6, "layout2" );

  mitkWidget1->show();
  mitkWidget2->show();
  mitkWidget3->hide();
  mitkWidget4->show();

  mitkWidget1->setMaximumSize(2000,1800);
  mitkWidget2->setMaximumSize(2000,1800);

  layout1->addWidget( mitkWidget1, 0, 0 );
  layout1->addWidget( mitkWidget2, 1, 0 );
  layout2->addLayout( layout1 );
  layout2->addWidget( mitkWidget4 );
  layout2->addWidget( levelWindowWidget );

  QmitkStdMultiWidgetLayout->addLayout( layout2 );

  m_Layout = LAYOUT_2X_2D_AND_3D_WIDGET;
}

void QmitkStdMultiWidget::changeLayoutToLeft2Dand3DRight2D()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2D and 3D left, 2D right Widget" << std::endl;

  QmitkStdMultiWidgetLayout = 
    new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QVBoxLayout *layout1 = new QVBoxLayout( 0, 0, 6, "layout1" );
  QHBoxLayout *layout2 = new QHBoxLayout( 0, 0, 6, "layout2" );

  mitkWidget1->show();
  mitkWidget2->show();
  mitkWidget3->hide();
  mitkWidget4->show();

  mitkWidget1->setMaximumSize(2000,2000);
  mitkWidget2->setMaximumSize(2000,2000);

  layout1->addWidget( mitkWidget1 );
  layout1->addWidget( mitkWidget4 );

  layout2->addLayout( layout1 );
  layout2->setStretchFactor( layout1, 1 );

  layout2->addWidget( mitkWidget2 );
  layout2->setStretchFactor( mitkWidget2, 2 );

  layout2->addWidget( levelWindowWidget );

  QmitkStdMultiWidgetLayout->addLayout( layout2 );

  m_Layout = LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET;
}

void QmitkStdMultiWidget::SetData( mitk::DataTreeIteratorBase* it )
{
  mitkWidget1->GetRenderer()->SetData(it);
  mitkWidget2->GetRenderer()->SetData(it);
  mitkWidget3->GetRenderer()->SetData(it);
  mitkWidget4->GetRenderer()->SetData(it);
}

void QmitkStdMultiWidget::Fit()
{
  vtkRenderer * vtkrenderer;
  mitkWidget1->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget2->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget3->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget4->GetRenderer()->GetDisplayGeometry()->Fit();

  int w=vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget1->GetRenderer()))
    ->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget2->GetRenderer()))
    ->GetVtkRenderer();

  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget3->GetRenderer()))
    ->GetVtkRenderer();

  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget4->GetRenderer()))
    ->GetVtkRenderer();

  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::AddPositionTrackingPointSet(
  mitk::DataTreeIteratorBase* treeIterator)
{
  if (treeIterator) m_Tree = treeIterator->GetTree(); 

  //PoinSetNode for MouseOrientation
  m_PositionTrackerNode = mitk::DataTreeNode::New();
  m_PositionTrackerNode->SetProperty(
    "name", new mitk::StringProperty("Mouse Position"));
  m_PositionTrackerNode->SetData( mitk::PointSet::New() );
  m_PositionTrackerNode->SetColor(1.0,0.33,0.0);
  m_PositionTrackerNode->SetProperty(
    "layer", new mitk::IntProperty(1001));
  m_PositionTrackerNode->SetProperty(
    "visible", new mitk::BoolProperty(true) );
  m_PositionTrackerNode->SetProperty(
    "inputdevice", new mitk::BoolProperty(true) );
  m_PositionTrackerNode->SetProperty(
    "BaseRendererMapperID", new mitk::IntProperty(0) );//point position 2D mouse
  m_PositionTrackerNode->SetProperty(
    "baserenderer", new mitk::StringProperty("N/A"));
  
}
void QmitkStdMultiWidget
::AddDisplayPlaneSubTree(mitk::DataTreeIteratorBase* it)
{
  // add the diplayed planes of the multiwidget to a node to which the subtree 
  // @a planesSubTree points ...

  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  node->SetProperty("name", new mitk::StringProperty("Widgets"));
  node->SetProperty("helper object", new mitk::BoolProperty(true));
  mitk::DataTreeIteratorClone dit = it;
  dit->Add(node);
  dit->GoToChild(dit->ChildPosition(node));

  float white[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  mitk::Geometry2DDataMapper2D::Pointer mapper;

  // ... of widget 1
  planeNode=mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitkWidget4->GetRenderer());
  planeNode->SetProperty("visible", new mitk::BoolProperty(true));
  planeNode->SetProperty("name", new mitk::StringProperty("widget1Plane"));
  planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
  planeNode->SetProperty("helper object", new mitk::BoolProperty(true));
  mapper = mitk::Geometry2DDataMapper2D::New();
  planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
  dit->Add(planeNode);
  mitk::DataTreeChildIterator childIterator(*(dit.GetPointer()));
  mapper->SetDataIteratorToOtherGeometry2Ds(&childIterator);

  // ... of widget 2
  planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitkWidget4->GetRenderer());
  planeNode->SetProperty("visible", new mitk::BoolProperty(true));
  planeNode->SetProperty("name", new mitk::StringProperty("widget2Plane"));
  planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
  planeNode->SetProperty("helper object", new mitk::BoolProperty(true));
  mapper = mitk::Geometry2DDataMapper2D::New();
  mapper->SetDataIteratorToOtherGeometry2Ds(&childIterator);
  planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
  dit->Add(planeNode);

  // ... of widget 3
  planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitkWidget4->GetRenderer());
  planeNode->SetProperty("visible", new mitk::BoolProperty(true));
  planeNode->SetProperty("name", new mitk::StringProperty("widget3Plane"));
  planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
  planeNode->SetProperty("helper object", new mitk::BoolProperty(true));
  mapper = mitk::Geometry2DDataMapper2D::New();
  mapper->SetDataIteratorToOtherGeometry2Ds(&childIterator);
  planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
  dit->Add(planeNode);

  planesIterator = dit;
}

mitk::SliceNavigationController* 
QmitkStdMultiWidget::GetTimeNavigationController()
{
  return timeNavigationController.GetPointer();
}


void QmitkStdMultiWidget::EnableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  mitk::DataTreeIteratorClone it = mitkWidget1->GetRenderer()->GetData();
  levelWindowWidget->setDataTree(dynamic_cast<mitk::DataTree*>(it->GetTree()));
  levelWindowWidget->show();
}


void QmitkStdMultiWidget::DisableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  levelWindowWidget->hide();
}


bool QmitkStdMultiWidget::InitializeStandardViews(
  mitk::DataTreeIteratorBase * it)
{
  bool boundingBoxInitialized = false;

  mitk::SliceNavigationController* sliceNavigatorTransversal =
    mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal = 
    mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal = 
    mitkWidget3->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorWidget4 = 
    mitkWidget4->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(
    mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(
    mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(
    mitk::SliceNavigationController::Frontal);
  sliceNavigatorWidget4->SetViewDirection(
    mitk::SliceNavigationController::Original);

  if ( it==NULL )
  {
    sliceNavigatorTransversal->Update();
    sliceNavigatorSagittal->Update();
    sliceNavigatorFrontal->Update();
    timeNavigationController->Update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    mitk::Geometry3D::Pointer geometry = mitk::DataTree::ComputeVisibleBoundingGeometry3D(it, NULL, "includeInBoundingBox");
    if ( geometry.IsNotNull() )
    {
      //lets see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1]<mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry = 
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(geometry, (unsigned int) duration);
        timegeometry->SetTimeBounds(timebounds); //@bug really required? FIXME

        timebounds[1] = timebounds[0]+1.0f;
        geometry->SetTimeBounds(timebounds);

        geometry=timegeometry;
      }

      double diagonalLength  = const_cast<mitk::BoundingBox*>(
        geometry->GetBoundingBox())->GetDiagonalLength2();

      if ( (diagonalLength > mitk::eps) && (diagonalLength < mitk::large) )
      {
        boundingBoxInitialized=true;
        
        // steppers are set so that the cross is centered in the image
        sliceNavigatorTransversal->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorTransversal->Update();
        sliceNavigatorTransversal->GetSlice()->SetPos( 
          sliceNavigatorTransversal->GetSlice()->GetSteps()/2 );

        sliceNavigatorSagittal->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorSagittal->Update();
        sliceNavigatorSagittal->GetSlice()->SetPos( 
          sliceNavigatorSagittal->GetSlice()->GetSteps()/2 );

        sliceNavigatorFrontal->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorFrontal->Update();
        sliceNavigatorFrontal->GetSlice()->SetPos( 
          sliceNavigatorFrontal->GetSlice()->GetSteps()/2 );

        sliceNavigatorWidget4->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorWidget4->Update();

        timeNavigationController->SetInputWorldGeometry(geometry.GetPointer());
        timeNavigationController->Update();

        // Tell observers that views are initialized now
        emit ViewsInitialized();

        // Temporary solution: Use ForceImmediateUpdate instead of RequestUpdate so
        // that Fit() resets the camera according to the new geometry.
        //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        this->Fit();
        mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
      }
    }
  }
  return boundingBoxInitialized;
}


bool QmitkStdMultiWidget
::InitializeStandardViews( const mitk::Geometry3D * geometry )
{
  bool boundingBoxInitialized = false;

  mitk::SliceNavigationController* sliceNavigatorTransversal =
    mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal =
    mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal =
    mitkWidget3->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(
    mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(
    mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(
    mitk::SliceNavigationController::Frontal);

  if ( (geometry==NULL) || (const_cast<mitk::BoundingBox*>(
    geometry->GetBoundingBox())->GetDiagonalLength2()<mitk::eps) )
  {
    sliceNavigatorTransversal->Update();
    sliceNavigatorSagittal->Update();
    sliceNavigatorFrontal->Update();
    timeNavigationController->Update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    mitk::Geometry3D::Pointer clonedgeometry = 
      static_cast<mitk::Geometry3D*>(geometry->Clone().GetPointer());

    // steppers are set so that the cross is centered in the image
    sliceNavigatorTransversal->SetInputWorldGeometry(
      clonedgeometry.GetPointer());
    sliceNavigatorTransversal->Update();
    sliceNavigatorTransversal->GetSlice()->SetPos( 
      sliceNavigatorTransversal->GetSlice()->GetSteps()/2 );

    sliceNavigatorSagittal->SetInputWorldGeometry(
      clonedgeometry.GetPointer());
    sliceNavigatorSagittal->Update();
    sliceNavigatorSagittal->GetSlice()->SetPos( 
      sliceNavigatorSagittal->GetSlice()->GetSteps()/2 );

    sliceNavigatorFrontal->SetInputWorldGeometry(
      clonedgeometry.GetPointer());
    sliceNavigatorFrontal->Update();
    sliceNavigatorFrontal->GetSlice()->SetPos( 
      sliceNavigatorFrontal->GetSlice()->GetSteps()/2 );

    timeNavigationController->SetInputWorldGeometry(
      clonedgeometry.GetPointer());
    timeNavigationController->Update();
    
    // Temporary solution: Use ForceImmediateUpdate instead of RequestUpdate so
    // that Fit() resets the camera according to the new geometry.
    //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    this->Fit();

    boundingBoxInitialized=true;
  }

  return boundingBoxInitialized;
}

void QmitkStdMultiWidget::RequestUpdate()
{
  mitkWidget1->GetRenderer()->GetRenderWindow()->RequestUpdate();
  mitkWidget2->GetRenderer()->GetRenderWindow()->RequestUpdate();
  mitkWidget3->GetRenderer()->GetRenderWindow()->RequestUpdate();
  mitkWidget4->GetRenderer()->GetRenderWindow()->RequestUpdate();
}

void QmitkStdMultiWidget::ForceImmediateUpdate()
{
  mitkWidget1->GetRenderer()->GetRenderWindow()->ForceImmediateUpdate();
  mitkWidget2->GetRenderer()->GetRenderWindow()->ForceImmediateUpdate();
  mitkWidget3->GetRenderer()->GetRenderWindow()->ForceImmediateUpdate();
  mitkWidget4->GetRenderer()->GetRenderWindow()->ForceImmediateUpdate();
}

void QmitkStdMultiWidget::wheelEvent( QWheelEvent * e )
{
  std::cout << "QmitkMultiWidget: grabbed mouse wheel event" << std::endl;
  emit WheelMoved( e );
}

mitk::DisplayVectorInteractor* QmitkStdMultiWidget::GetMoveAndZoomInteractor()
{
  return m_MoveAndZoomInteractor.GetPointer();
}

void QmitkStdMultiWidget::ReInitializeStandardViews()
{
  mitk::SliceNavigationController* sliceNavigatorTransversal = 
    mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal = 
    mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal = 
    mitkWidget3->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(
    mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(
    mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(
    mitk::SliceNavigationController::Frontal);

  sliceNavigatorTransversal->Update();
  sliceNavigatorSagittal->Update();
  sliceNavigatorFrontal->Update();
  timeNavigationController->Update();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow1() const
{
  return mitkWidget1->GetRenderWindow();
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow2() const
{
  return mitkWidget2->GetRenderWindow();
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow3() const
{
  return mitkWidget3->GetRenderWindow();
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow4() const
{
  return mitkWidget4->GetRenderWindow();
}

const mitk::Point3D &
QmitkStdMultiWidget::GetLastLeftClickPosition() const
{
  return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}

const mitk::Point3D
QmitkStdMultiWidget::GetCrossPosition() const
{
  const mitk::PlaneGeometry *plane1 = 
    mitkWidget1->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane2 = 
    mitkWidget2->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  const mitk::PlaneGeometry *plane3 = 
    mitkWidget3->GetSliceNavigationController()->GetCurrentPlaneGeometry();

  mitk::Line3D line;
  if ( (plane1 != NULL) && (plane2 != NULL)
    && (plane1->IntersectionLine( plane2, line )) )
  {
    mitk::Point3D point;
    if ( (plane3 != NULL) 
      && (plane3->IntersectionPoint( line, point )) )
    {
      return point;
    }
  }
  return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}

void QmitkStdMultiWidget::EnablePositionTracking()
{
  if(!m_PositionTracker)
  {
    m_PositionTracker = mitk::PositionTracker::New("PositionTracker", NULL);
  }

  mitk::GlobalInteraction* globalInteraction = 
    mitk::GlobalInteraction::GetInstance();

  if(globalInteraction)
  {
     mitk::DataTreePreOrderIterator iterator(m_Tree);
     iterator.Add( m_PositionTrackerNode);
     globalInteraction->AddListener(m_PositionTracker); 
  }
}

void QmitkStdMultiWidget::DisablePositionTracking()
{
  mitk::GlobalInteraction* globalInteraction =
    mitk::GlobalInteraction::GetInstance();

  if(globalInteraction)
  {
    mitk::DataTreePreOrderIterator it(m_Tree);

    while ( !it.IsAtEnd() )
    {
      if(it.Get() == m_PositionTrackerNode.GetPointer() )
      {
        it.Disconnect();
        break;
      }
      ++it;
    }

    globalInteraction->RemoveListener(m_PositionTracker); 
  }
}

void QmitkStdMultiWidget::EnsureDisplayContainsPoint(
  mitk::DisplayGeometry* displayGeometry, const mitk::Point3D& p)
{
  mitk::Point2D pointOnPlane;
  displayGeometry->Map( p, pointOnPlane );

  // point minus origin < width or height ==> outside ?
  mitk::Vector2D pointOnRenderWindow_MM;
  pointOnRenderWindow_MM = pointOnPlane.GetVectorFromOrigin() 
    - displayGeometry->GetOriginInMM();
  
  mitk::Vector2D sizeOfDisplay( displayGeometry->GetSizeInMM() );

  if (   sizeOfDisplay[0] < pointOnRenderWindow_MM[0] 
      ||                0 > pointOnRenderWindow_MM[0]
      || sizeOfDisplay[1] < pointOnRenderWindow_MM[1]
      ||                0 > pointOnRenderWindow_MM[1] )
  {
    // point is not visible -> move geometry
    mitk::Vector2D offset( (pointOnRenderWindow_MM - sizeOfDisplay / 2.0) 
      / displayGeometry->GetScaleFactorMMPerDisplayUnit() );
    
    displayGeometry->MoveBy( offset );
  }
}

void QmitkStdMultiWidget::MoveCrossToPosition(
  const mitk::Point3D& newPosition)
{
  // create a PositionEvent with the given position and
  // tell the slice navigation controllers to move there

  mitk::Point2D p2d;
  mitk::PositionEvent event( mitkWidget1->GetRenderer(), 0, 0, 0, 
    mitk::Key_unknown, p2d, newPosition );
  mitk::StateEvent stateEvent(mitk::EIDLEFTMOUSEBTN, &event);
  mitk::StateEvent stateEvent2(mitk::EIDRIGHTMOUSEBTN, &event);

  if (m_SlicesRotation)
  {
    m_SlicesRotator->HandleEvent( &stateEvent );
    
    // just in case SNCs will develop something that depends on the mouse 
    // button being released again
    m_SlicesRotator->HandleEvent( &stateEvent2 );
  }
  else
  {
    mitkWidget1->GetSliceNavigationController()->HandleEvent( &stateEvent );
    mitkWidget2->GetSliceNavigationController()->HandleEvent( &stateEvent );
    mitkWidget3->GetSliceNavigationController()->HandleEvent( &stateEvent );
    
    // just in case SNCs will develop something that depends on the mouse
    // button being released again
    mitkWidget1->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    mitkWidget2->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    mitkWidget3->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
  }
  
  // determine if cross is now out of display
  // if so, move the display window
  EnsureDisplayContainsPoint( mitkWidget1->GetRenderer()
    ->GetDisplayGeometry(), newPosition );
  EnsureDisplayContainsPoint( mitkWidget2->GetRenderer()
    ->GetDisplayGeometry(), newPosition );
  EnsureDisplayContainsPoint( mitkWidget3->GetRenderer()
    ->GetDisplayGeometry(), newPosition );

  // update displays
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkStdMultiWidget::EnableNavigationControllerEventListening()
{
  // Let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction* globalInteraction = 
    mitk::GlobalInteraction::GetInstance();
  
  if (m_SlicesRotation)
  {
    globalInteraction->AddListener( m_SlicesRotator );
  }
  else
  {
    globalInteraction->AddListener( mitkWidget1->GetSliceNavigationController() );
    globalInteraction->AddListener( mitkWidget2->GetSliceNavigationController() );
    globalInteraction->AddListener( mitkWidget3->GetSliceNavigationController() );
    globalInteraction->AddListener( mitkWidget4->GetSliceNavigationController() );
  }
    
  globalInteraction->AddListener( timeNavigationController );
}

void QmitkStdMultiWidget::DisableNavigationControllerEventListening()
{
  // Do not let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction* globalInteraction = 
    mitk::GlobalInteraction::GetInstance();

  if (m_SlicesRotation)
  {
    globalInteraction->RemoveListener( m_SlicesRotator );
  }
  else
  {
    globalInteraction->RemoveListener(
      mitkWidget1->GetSliceNavigationController());
    globalInteraction->RemoveListener( 
      mitkWidget2->GetSliceNavigationController());
    globalInteraction->RemoveListener( 
      mitkWidget3->GetSliceNavigationController());
    globalInteraction->RemoveListener( 
      mitkWidget4->GetSliceNavigationController());
  }
  
  globalInteraction->RemoveListener( timeNavigationController );
}

void QmitkStdMultiWidget::EnableSliceRotation(bool on)
{
  if (on == m_SlicesRotation) return;

  m_SlicesRotation = on;
  // @ TODO add bool m_NavigationControllerListeningEnable!
  mitk::GlobalInteraction* globalInteraction = 
    mitk::GlobalInteraction::GetInstance();
  if (on)
  {
    globalInteraction->RemoveListener( 
      mitkWidget1->GetSliceNavigationController());
    globalInteraction->RemoveListener(
      mitkWidget2->GetSliceNavigationController());
    globalInteraction->RemoveListener(
      mitkWidget3->GetSliceNavigationController());
    globalInteraction->RemoveListener(
      mitkWidget4->GetSliceNavigationController());
    globalInteraction->AddListener( m_SlicesRotator );
  }
  else
  {
    globalInteraction->RemoveListener( m_SlicesRotator );
    globalInteraction->AddListener( 
      mitkWidget1->GetSliceNavigationController() );
    globalInteraction->AddListener(
      mitkWidget2->GetSliceNavigationController() );
    globalInteraction->AddListener(
      mitkWidget3->GetSliceNavigationController() );
    globalInteraction->AddListener(
      mitkWidget4->GetSliceNavigationController() );

    ReInitializeStandardViews();
  }
}

int QmitkStdMultiWidget::GetLayout() const
{
  return m_Layout;
}


void QmitkStdMultiWidget::EnableGradientBackground()
{
  // gradient background is by default only in widget 4, otherwise
  // interferences between 2D rendering and VTK rendering may occur. 
  //m_GradientBackground1->Enable();
  //m_GradientBackground2->Enable();
  //m_GradientBackground3->Enable();
  m_GradientBackground4->Enable();
}


void QmitkStdMultiWidget::DisableGradientBackground()
{
  //m_GradientBackground1->Disable();
  //m_GradientBackground2->Disable();
  //m_GradientBackground3->Disable();
  m_GradientBackground4->Disable();
}

void QmitkStdMultiWidget::EnableDepartmentLogo()
{
  m_LogoRendering4->Enable();
}

void QmitkStdMultiWidget::DisableDepartmentLogo()
{
  m_LogoRendering4->Disable();
}

mitk::SlicesRotator * QmitkStdMultiWidget::GetSlicesRotator() const
{
  return m_SlicesRotator;
}

void QmitkStdMultiWidget::SetWidgetPlaneVisibility(const char* widgetName, bool visible)
{
  mitk::DataTree* tree = dynamic_cast<mitk::DataTree*>( m_Tree.GetPointer() );
  if (!tree) return;

  mitk::DataTreeIteratorClone it = tree->GetNext("name", new mitk::StringProperty(widgetName));
  if (!it->IsAtEnd())
  {
    mitk::DataTreeNode::Pointer node = it->Get();
    if ( node.IsNotNull() )
    {
      node->SetVisibility(visible);
    }
  }
}
 
void QmitkStdMultiWidget::SetWidgetPlanesVisibility(bool visible)
{
  SetWidgetPlaneVisibility("widget1Plane", visible);
  SetWidgetPlaneVisibility("widget2Plane", visible);
  SetWidgetPlaneVisibility("widget3Plane", visible);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  emit WidgetPlanesVisibilityChanged(visible);
}

void QmitkStdMultiWidget::SetWidgetPlanesLocked(bool locked)
{
  //do your job and lock or unlock slices.
  GetRenderWindow1()->GetSliceNavigationController()->SetSliceLocked(locked);
  GetRenderWindow2()->GetSliceNavigationController()->SetSliceLocked(locked);
  GetRenderWindow3()->GetSliceNavigationController()->SetSliceLocked(locked);
  
  emit WidgetPlanesLockedChanged(locked);
}

void QmitkStdMultiWidget::SetWidgetPlanesRotationLocked(bool locked)
{
  //do your job and lock or unlock slices.
  GetRenderWindow1()->GetSliceNavigationController()->SetSliceRotationLocked(locked);
  GetRenderWindow2()->GetSliceNavigationController()->SetSliceRotationLocked(locked);
  GetRenderWindow3()->GetSliceNavigationController()->SetSliceRotationLocked(locked);

  emit WidgetPlanesRotationLockedChanged(locked);
}

void QmitkStdMultiWidget::SetWidgetPlanesRotationLinked( bool link )
{
  m_SlicesRotator->SetLinkPlanes( link );
  emit WidgetPlanesRotationLinked( link );
}

void QmitkStdMultiWidget::SetWidgetPlanesRotationEnabled( bool on )
{
  EnableSliceRotation( on );
  emit WidgetPlanesRotationEnabled(on);
}


void QmitkStdMultiWidget::SetGradientBackgroundColors( const mitk::Color & upper, const mitk::Color & lower )
{
  m_GradientBackground1->SetGradientColors(upper[0], upper[1], upper[2], lower[0], lower[1], lower[2]);
  m_GradientBackground2->SetGradientColors(upper[0], upper[1], upper[2], lower[0], lower[1], lower[2]);
  m_GradientBackground3->SetGradientColors(upper[0], upper[1], upper[2], lower[0], lower[1], lower[2]);
  m_GradientBackground4->SetGradientColors(upper[0], upper[1], upper[2], lower[0], lower[1], lower[2]);
}



void QmitkStdMultiWidget::SetDepartmentLogoPath( const char * path )
{
  m_LogoRendering1->SetLogoSource(path);
  m_LogoRendering2->SetLogoSource(path);
  m_LogoRendering3->SetLogoSource(path);
  m_LogoRendering4->SetLogoSource(path);
}
