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
#include "mitkVtkPropRenderer.h"
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
  
  planesIterator = NULL;
  m_PositionTracker = NULL;

  // transfer colors in WorldGeometry-Nodes of the associated Renderer
  QColor qcolor;
  //float color[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  mitk::IntProperty::Pointer  layer;

  // of widget 1
  planeNode=mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(1.0,0.0,0.0);
  layer = mitk::IntProperty::New(1000);
  planeNode->SetProperty("layer",layer);

  // ... of widget 2
  planeNode=mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(0.0,1.0,0.0);
  layer = mitk::IntProperty::New(1000);
  planeNode->SetProperty("layer",layer);

  // ... of widget 3
  planeNode=mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(0.0,0.0,1.0);
  layer = mitk::IntProperty::New(1000);
  planeNode->SetProperty("layer",layer);

  // ... of widget 4
  planeNode=mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(1.0,1.0,0.0);
  layer = mitk::IntProperty::New(1000);
  planeNode->SetProperty("layer",layer);

  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->SetMapperID(mitk::BaseRenderer::Standard3D);
  // Set plane mode (slicing/rotation behavior) to slicing (default)
  m_PlaneMode = PLANE_MODE_SLICING;

  // Set default view directions for SNCs
  mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Transversal );
  mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Sagittal );
  mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Frontal );
  mitkWidget4->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Original );

  // create a slice rotator
  // m_SlicesRotator = mitk::SlicesRotator::New();
  // @TODO next line causes sure memory leak
  // rotator will be created nonetheless (will be switched on and off)
  m_SlicesRotator = new mitk::SlicesRotator("slices-rotator");
  m_SlicesRotator->AddSliceController( 
    mitkWidget1->GetSliceNavigationController() );
  m_SlicesRotator->AddSliceController( 
    mitkWidget2->GetSliceNavigationController() );
  m_SlicesRotator->AddSliceController(
    mitkWidget3->GetSliceNavigationController() );

  // create a slice swiveller (using the same state-machine as SlicesRotator)
  m_SlicesSwiveller = new mitk::SlicesSwiveller("slices-rotator");
  m_SlicesSwiveller->AddSliceController( 
    mitkWidget1->GetSliceNavigationController() );
  m_SlicesSwiveller->AddSliceController( 
    mitkWidget2->GetSliceNavigationController() );
  m_SlicesSwiveller->AddSliceController(
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
    ->ConnectGeometrySendEvent(mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));

  // Set TimeNavigationController to RenderingManager 
  // (which uses it internally for views initialization!)
  mitk::RenderingManager::GetInstance()->SetTimeNavigationController(
    timeNavigationController );

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
    mitkWidget1->GetRenderWindow() );
  m_GradientBackground1->Disable();
  
  m_GradientBackground2 = mitk::GradientBackground::New();
  m_GradientBackground2->SetRenderWindow(
    mitkWidget2->GetRenderWindow() );
  m_GradientBackground2->Disable();

  m_GradientBackground3 = mitk::GradientBackground::New();
  m_GradientBackground3->SetRenderWindow(
    mitkWidget3->GetRenderWindow() );
  m_GradientBackground3->Disable();

  m_GradientBackground4 = mitk::GradientBackground::New();
  m_GradientBackground4->SetRenderWindow(
    mitkWidget4->GetRenderWindow() );
  m_GradientBackground4->SetGradientColors(0.0,0.1,0.3,0.7,0.7,0.8);
  m_GradientBackground4->Enable();

  // setup the department logo rendering
  m_LogoRendering1 = mitk::LogoRendering::New();
  m_LogoRendering1->SetRenderWindow(
    mitkWidget1->GetRenderWindow() );
  m_LogoRendering1->Disable();

  m_LogoRendering2 = mitk::LogoRendering::New();
  m_LogoRendering2->SetRenderWindow(
    mitkWidget2->GetRenderWindow() );
  m_LogoRendering2->Disable();

  m_LogoRendering3 = mitk::LogoRendering::New();
  m_LogoRendering3->SetRenderWindow(
    mitkWidget3->GetRenderWindow() );
  m_LogoRendering3->Disable();
  
  m_LogoRendering4 = mitk::LogoRendering::New();
  m_LogoRendering4->SetRenderWindow(
    mitkWidget4->GetRenderWindow() );
  m_LogoRendering4->Enable();

  m_RectangleRendering1 = mitk::ColoredRectangleRendering::New();
  m_RectangleRendering1->SetRenderWindow(
    mitkWidget1->GetRenderWindow() );
  m_RectangleRendering1->Enable(1.0,0.0,0.0);
  
  m_RectangleRendering2 = mitk::ColoredRectangleRendering::New();
  m_RectangleRendering2->SetRenderWindow(
    mitkWidget2->GetRenderWindow() );
  m_RectangleRendering2->Enable(0.0,1.0,0.0);
  
  m_RectangleRendering3 = mitk::ColoredRectangleRendering::New();
  m_RectangleRendering3->SetRenderWindow(
    mitkWidget3->GetRenderWindow() );
  m_RectangleRendering3->Enable(0.0,0.0,1.0);
  
  m_RectangleRendering4 = mitk::ColoredRectangleRendering::New();
  m_RectangleRendering4->SetRenderWindow(
    mitkWidget4->GetRenderWindow() );
  m_RectangleRendering4->Enable(1.0,1.0,0.0);
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
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->SetData(it);
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->SetData(it);
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->SetData(it);
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->SetData(it);
}

void QmitkStdMultiWidget::Fit()
{
  vtkRenderer * vtkrenderer;
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetDisplayGeometry()->Fit();

  int w=vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow())->GetVtkRenderer();
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
    "name", mitk::StringProperty::New("Mouse Position"));
  m_PositionTrackerNode->SetData( mitk::PointSet::New() );
  m_PositionTrackerNode->SetColor(1.0,0.33,0.0);
  m_PositionTrackerNode->SetProperty(
    "layer", mitk::IntProperty::New(1001));
  m_PositionTrackerNode->SetProperty(
    "visible", mitk::BoolProperty::New(true) );
  m_PositionTrackerNode->SetProperty(
    "inputdevice", mitk::BoolProperty::New(true) );
  m_PositionTrackerNode->SetProperty(
    "BaseRendererMapperID", mitk::IntProperty::New(0) );//point position 2D mouse
  m_PositionTrackerNode->SetProperty(
    "baserenderer", mitk::StringProperty::New("N/A"));
  
}
void QmitkStdMultiWidget
::AddDisplayPlaneSubTree(mitk::DataTreeIteratorBase* it)
{
  // add the diplayed planes of the multiwidget to a node to which the subtree 
  // @a planesSubTree points ...

  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  node->SetProperty("name", mitk::StringProperty::New("Widgets"));
  node->SetProperty("helper object", mitk::BoolProperty::New(true));
  mitk::DataTreeIteratorClone dit = it;
  dit->Add(node);
  dit->GoToChild(dit->ChildPosition(node));

  float white[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  mitk::Geometry2DDataMapper2D::Pointer mapper;

  // ... of widget 1
  planeNode=(mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow()))->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));
  planeNode->SetProperty("visible", mitk::BoolProperty::New(true));
  planeNode->SetProperty("name", mitk::StringProperty::New("widget1Plane"));
  planeNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  planeNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::Geometry2DDataMapper2D::New();
  planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
  dit->Add(planeNode);
  mitk::DataTreeChildIterator childIterator(*(dit.GetPointer()));
  mapper->SetDataIteratorToOtherGeometry2Ds(&childIterator);

  // ... of widget 2
  planeNode=(mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow()))->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));
  planeNode->SetProperty("visible", mitk::BoolProperty::New(true));
  planeNode->SetProperty("name", mitk::StringProperty::New("widget2Plane"));
  planeNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  planeNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  mapper = mitk::Geometry2DDataMapper2D::New();
  mapper->SetDataIteratorToOtherGeometry2Ds(&childIterator);
  planeNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
  dit->Add(planeNode);

  // ... of widget 3
  planeNode=(mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow()))->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));
  planeNode->SetProperty("visible", mitk::BoolProperty::New(true));
  planeNode->SetProperty("name", mitk::StringProperty::New("widget3Plane"));
  planeNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  planeNode->SetProperty("helper object", mitk::BoolProperty::New(true));
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
  mitk::DataTreeIteratorClone it = mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())->GetData();
  levelWindowWidget->setDataTree(dynamic_cast<mitk::DataTree*>(it->GetTree()));
  levelWindowWidget->show();
}


void QmitkStdMultiWidget::DisableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  levelWindowWidget->hide();
}


// CAUTION: Legacy code for enabling Qt-signal-controlled view initialization.
// Use RenderingManager::InitializeViews() instead.
bool QmitkStdMultiWidget::InitializeStandardViews(
  mitk::DataTreeIteratorBase * it)
{
  return mitk::RenderingManager::GetInstance()->InitializeViews( it );
}


// CAUTION: Legacy code for enabling Qt-signal-controlled view initialization.
// Use RenderingManager::InitializeViews() instead.
bool QmitkStdMultiWidget
::InitializeStandardViews( const mitk::Geometry3D * geometry )
{
  return mitk::RenderingManager::GetInstance()->InitializeViews( geometry );
}

void QmitkStdMultiWidget::RequestUpdate()
{
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitkWidget1->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitkWidget2->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitkWidget3->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(mitkWidget4->GetRenderWindow());
}

void QmitkStdMultiWidget::ForceImmediateUpdate()
{
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(mitkWidget1->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(mitkWidget2->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(mitkWidget3->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(mitkWidget4->GetRenderWindow());
}

void QmitkStdMultiWidget::wheelEvent( QWheelEvent * e )
{
  emit WheelMoved( e );
}

mitk::DisplayVectorInteractor* QmitkStdMultiWidget::GetMoveAndZoomInteractor()
{
  return m_MoveAndZoomInteractor.GetPointer();
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow1() const
{
  return mitkWidget1;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow2() const
{
  return mitkWidget2;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow3() const
{
  return mitkWidget3;
}

QmitkRenderWindow* QmitkStdMultiWidget::GetRenderWindow4() const
{
  return mitkWidget4;
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
  mitk::PositionEvent event( mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow()), 0, 0, 0, 
    mitk::Key_unknown, p2d, newPosition );
  mitk::StateEvent stateEvent(mitk::EIDLEFTMOUSEBTN, &event);
  mitk::StateEvent stateEvent2(mitk::EIDLEFTMOUSERELEASE, &event);

  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    mitkWidget1->GetSliceNavigationController()->HandleEvent( &stateEvent );
    mitkWidget2->GetSliceNavigationController()->HandleEvent( &stateEvent );
    mitkWidget3->GetSliceNavigationController()->HandleEvent( &stateEvent );
    
    // just in case SNCs will develop something that depends on the mouse
    // button being released again
    mitkWidget1->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    mitkWidget2->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    mitkWidget3->GetSliceNavigationController()->HandleEvent( &stateEvent2 );
    break;

  case PLANE_MODE_ROTATION:
    m_SlicesRotator->HandleEvent( &stateEvent );
    
    // just in case SNCs will develop something that depends on the mouse 
    // button being released again
    m_SlicesRotator->HandleEvent( &stateEvent2 );
    break;

  case PLANE_MODE_SWIVEL:
    m_SlicesSwiveller->HandleEvent( &stateEvent );
    
    // just in case SNCs will develop something that depends on the mouse 
    // button being released again
    m_SlicesSwiveller->HandleEvent( &stateEvent2 );
    break;
  }
  
  // determine if cross is now out of display
  // if so, move the display window
  EnsureDisplayContainsPoint( mitk::BaseRenderer::GetInstance(mitkWidget1->GetRenderWindow())
    ->GetDisplayGeometry(), newPosition );
  EnsureDisplayContainsPoint( mitk::BaseRenderer::GetInstance(mitkWidget2->GetRenderWindow())
    ->GetDisplayGeometry(), newPosition );
  EnsureDisplayContainsPoint( mitk::BaseRenderer::GetInstance(mitkWidget3->GetRenderWindow())
    ->GetDisplayGeometry(), newPosition );

  // update displays
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkStdMultiWidget::EnableNavigationControllerEventListening()
{
  // Let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();
  
  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    gi->AddListener( mitkWidget1->GetSliceNavigationController() );
    gi->AddListener( mitkWidget2->GetSliceNavigationController() );
    gi->AddListener( mitkWidget3->GetSliceNavigationController() );
    gi->AddListener( mitkWidget4->GetSliceNavigationController() );
    break;

  case PLANE_MODE_ROTATION:
    gi->AddListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    gi->AddListener( m_SlicesSwiveller );
    break;
  }

  gi->AddListener( timeNavigationController );
}

void QmitkStdMultiWidget::DisableNavigationControllerEventListening()
{
  // Do not let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();

  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    gi->RemoveListener( mitkWidget1->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget2->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget3->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget4->GetSliceNavigationController() );
    break;

  case PLANE_MODE_ROTATION:
    gi->RemoveListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    gi->RemoveListener( m_SlicesSwiveller );
    break;
  }

  gi->RemoveListener( timeNavigationController );
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

mitk::SlicesSwiveller * QmitkStdMultiWidget::GetSlicesSwiveller() const
{
  return m_SlicesSwiveller;
}

void QmitkStdMultiWidget::SetWidgetPlaneVisibility(const char* widgetName, bool visible)
{
  mitk::DataTree* tree = dynamic_cast<mitk::DataTree*>( m_Tree.GetPointer() );
  if (!tree) return;

  mitk::DataTreeIteratorClone it = tree->GetNext("name", mitk::StringProperty::New(widgetName));
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
  m_SlicesSwiveller->SetLinkPlanes( link );
  emit WidgetPlanesRotationLinked( link );
}

void QmitkStdMultiWidget::SetWidgetPlaneMode( int mode )
{
  // Do nothing if mode didn't change
  if ( m_PlaneMode == mode )
  {
    return;
  }

  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();

  // Remove listeners of previous mode
  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    // Notify MainTemplate GUI that this mode has been deselected
    emit WidgetPlaneModeSlicing( false );

    gi->RemoveListener( mitkWidget1->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget2->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget3->GetSliceNavigationController() );
    gi->RemoveListener( mitkWidget4->GetSliceNavigationController() );
    break;

  case PLANE_MODE_ROTATION:
    // Notify MainTemplate GUI that this mode has been deselected
    emit WidgetPlaneModeRotation( false );

    gi->RemoveListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    // Notify MainTemplate GUI that this mode has been deselected
    emit WidgetPlaneModeSwivel( false );

    gi->RemoveListener( m_SlicesSwiveller );
    break;
  }


  // Set new mode and add corresponding listener to GlobalInteraction
  m_PlaneMode = mode;

  switch ( m_PlaneMode )
  {
  default:
  case PLANE_MODE_SLICING:
    // Notify MainTemplate GUI that this mode has been selected
    emit WidgetPlaneModeSlicing( true );

    // Add listeners
    gi->AddListener( mitkWidget1->GetSliceNavigationController() );
    gi->AddListener( mitkWidget2->GetSliceNavigationController() );
    gi->AddListener( mitkWidget3->GetSliceNavigationController() );
    gi->AddListener( mitkWidget4->GetSliceNavigationController() );

    mitk::RenderingManager::GetInstance()->InitializeViews();
    break;

  case PLANE_MODE_ROTATION:
    // Notify MainTemplate GUI that this mode has been selected
    emit WidgetPlaneModeRotation( true );

    // Add listener
    gi->AddListener( m_SlicesRotator );
    break;

  case PLANE_MODE_SWIVEL:
    // Notify MainTemplate GUI that this mode has been selected
    emit WidgetPlaneModeSwivel( true );

    // Add listener
    gi->AddListener( m_SlicesSwiveller );
    break;
  }
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


void QmitkStdMultiWidget::SetWidgetPlaneModeToSlicing( bool activate )
{
  if ( activate )
  {
    this->SetWidgetPlaneMode( PLANE_MODE_SLICING );
  }
}


void QmitkStdMultiWidget::SetWidgetPlaneModeToRotation( bool activate )
{
  if ( activate )
  {
    this->SetWidgetPlaneMode( PLANE_MODE_ROTATION );
  }
}


void QmitkStdMultiWidget::SetWidgetPlaneModeToSwivel( bool activate )
{
  if ( activate )
  {
    this->SetWidgetPlaneMode( PLANE_MODE_SWIVEL );
  }
}
