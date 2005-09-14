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
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkGeometry2DDataVtkMapper3D.h>
#include <vtkRenderer.h>
#include <mitkOpenGLRenderer.h>
// for AdjustCross
#include <mitkPositionEvent.h>
#include <mitkInteractionConst.h>
#include <mitkGlobalInteraction.h>

void QmitkStdMultiWidget::init()
{
  mitkWidget1->setPaletteBackgroundColor ("red");
  mitkWidget2->setPaletteBackgroundColor ("green");
  mitkWidget3->setPaletteBackgroundColor ("blue");
  mitkWidget4->setPaletteBackgroundColor ("yellow");

  planesIterator = NULL;

  //transfer colors in WorldGeometry-Nodes of the associated Renderer
  QColor qcolor;
  float color[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  mitk::IntProperty::Pointer  layer;
  // ... of widget 1
  qcolor=mitkWidget1->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);
  // ... of widget 2
  qcolor=mitkWidget2->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);
  // ... of widget 3
  qcolor=mitkWidget3->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);
  // ... of widget 4
  qcolor=mitkWidget4->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget4->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1000);
  planeNode->SetProperty("layer",layer);

  //initialize timeNavigationController: send time via sliceNavigationControllers
  timeNavigationController = new mitk::SliceNavigationController(NULL);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget1->GetSliceNavigationController(), false);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget2->GetSliceNavigationController(), false);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget3->GetSliceNavigationController(), false);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget4->GetSliceNavigationController(), false);
  mitkWidget1->GetSliceNavigationController()->ConnectGeometrySendEvent(mitkWidget4->GetRenderer());

  //reverse connection between sliceNavigationControllers and timeNavigationController
  mitkWidget1->GetSliceNavigationController()->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);
  mitkWidget2->GetSliceNavigationController()->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);
  mitkWidget3->GetSliceNavigationController()->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);
  mitkWidget4->GetSliceNavigationController()->ConnectGeometryTimeEvent(timeNavigationController.GetPointer(), false);

  //initialize update-controller
  multiplexUpdateController = new mitk::MultiplexUpdateController("navigation");
  multiplexUpdateController->AddRenderWindow(mitkWidget1->GetRenderer()->GetRenderWindow());
  multiplexUpdateController->AddRenderWindow(mitkWidget2->GetRenderer()->GetRenderWindow());
  multiplexUpdateController->AddRenderWindow(mitkWidget3->GetRenderer()->GetRenderWindow());
  multiplexUpdateController->AddRenderWindow(mitkWidget4->GetRenderer()->GetRenderWindow());

  //connect sliceNavigationControllers to update-controller
  mitkWidget1->GetSliceNavigationController()->ConnectUpdateRequest(multiplexUpdateController.GetPointer());
  mitkWidget2->GetSliceNavigationController()->ConnectUpdateRequest(multiplexUpdateController.GetPointer());
  mitkWidget3->GetSliceNavigationController()->ConnectUpdateRequest(multiplexUpdateController.GetPointer());
  mitkWidget4->GetSliceNavigationController()->ConnectUpdateRequest(multiplexUpdateController.GetPointer());
  
  timeNavigationController->ConnectUpdateRequest(multiplexUpdateController.GetPointer());

  // instantiate display interactor
  m_MoveAndZoomInteractor = new mitk::DisplayVectorInteractor("moveNzoom", new mitk::DisplayInteractor() );
  
  m_LastLeftClickPositionSupplier = new mitk::CoordinateSupplier("navigation", NULL);
  mitk::GlobalInteraction::GetGlobalInteraction()->AddListener(m_LastLeftClickPositionSupplier);
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesUp()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2D images up... " << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0,02, "QmitkStdMultiWidgetLayout");

  QHBoxLayout  * layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  QGridLayout *layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");

  QHBoxLayout *TwoDLayout = new QHBoxLayout( 0, 0, 6, "2DLayout");

  mitkWidget1->setMaximumSize(2000,200);
  mitkWidget2->setMaximumSize(2000,200);
  mitkWidget3->setMaximumSize(2000,200);

  TwoDLayout ->addWidget( mitkWidget1 );
  TwoDLayout ->addWidget( mitkWidget2 );
  TwoDLayout ->addWidget( mitkWidget3 );

  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout3->addLayout( TwoDLayout , 0, 0 );
  layout3->addWidget( mitkWidget4, 1, 0 );

  layout4->addLayout( layout3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesLeft()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2D images left... " << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0,0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  QGridLayout *layout3 = new QGridLayout( 0, 1, 2, 0, 6, "layout3");

  QVBoxLayout *TwoDLayout = new QVBoxLayout( 0, 0, 6, "2DLayout");

  mitkWidget1->setMaximumSize(300,300);
  mitkWidget2->setMaximumSize(300,300);
  mitkWidget3->setMaximumSize(300,300);

  TwoDLayout ->addWidget( mitkWidget1 );
  TwoDLayout ->addWidget( mitkWidget2 );
  TwoDLayout ->addWidget( mitkWidget3 );

  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout3->addLayout( TwoDLayout , 0, 0 );
  layout3->addWidget( mitkWidget4, 0,1 );

  layout4->addLayout( layout3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );

}

void QmitkStdMultiWidget::changeLayoutToDefault()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to default... " << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

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
}


void QmitkStdMultiWidget::changeLayoutToBig3D()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big 3D ..." << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
  mitkWidget1->hide();
  mitkWidget2->hide();
  mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  layout4->addWidget( mitkWidget4 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
}

void QmitkStdMultiWidget::changeLayoutToWidget1()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big Widget1 ..." << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->setMaximumSize(2000,2000);

  mitkWidget2->hide();
  mitkWidget3->hide();
  mitkWidget4->hide();
  if ( mitkWidget1->isHidden() ) mitkWidget1->show();

  layout4->addWidget( mitkWidget1 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
}


void QmitkStdMultiWidget::changeLayoutToWidget2()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big Widget2 ..." << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget2->setMaximumSize(2000,2000);

  mitkWidget1->hide();
  mitkWidget3->hide();
  mitkWidget4->hide();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();

  layout4->addWidget( mitkWidget2 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
}

void QmitkStdMultiWidget::changeLayoutToWidget3()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to big Widget1 ..." << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget3->setMaximumSize(2000,2000);

  mitkWidget1->hide();
  mitkWidget2->hide();
  mitkWidget4->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();

  layout4->addWidget( mitkWidget3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
}

void QmitkStdMultiWidget::changeLayoutTo2x2Dand3DWidget()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to 2 x 2D and 3D Widget" << std::endl;

  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

  QVBoxLayout *layout1 = new QVBoxLayout( );
  QHBoxLayout *layout2 = new QHBoxLayout( );

  mitkWidget1->setMaximumSize(2000,2000);
  mitkWidget2->setMaximumSize(2000,2000);

  layout1->addWidget( mitkWidget1 );
  layout1->addWidget( mitkWidget2 );
  layout2->addLayout( layout1 );
  layout2->addWidget( mitkWidget4 );
  layout2->addWidget( levelWindowWidget );

  if ( mitkWidget1->isHidden() ) mitkWidget1->show();
  if ( mitkWidget2->isHidden() ) mitkWidget2->show();
  if ( !mitkWidget3->isHidden() ) mitkWidget3->hide();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  QmitkStdMultiWidgetLayout->addLayout( layout2 );
}

void QmitkStdMultiWidget::changeLayoutToRowWidget3And4()
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to Widget3 and 4 in a Row..." << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  mitkWidget3->setMaximumSize(2000,2000);
  mitkWidget4->setMaximumSize(2000,2000);

  layout4 ->addWidget( mitkWidget3);
  layout4 ->addWidget( mitkWidget4);
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
}


void QmitkStdMultiWidget::changeLayoutToColumnWidget3And4()//doesn't work yet
{
  delete QmitkStdMultiWidgetLayout ;

  std::cout << "changing layout to Widget3 and 4 in one Column..." << std::endl;
  QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");
  QGridLayout *layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");
  QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");

  mitkWidget1->hide();
  mitkWidget2->hide();
  if ( mitkWidget3->isHidden() ) mitkWidget3->show();
  if ( mitkWidget4->isHidden() ) mitkWidget4->show();

  mitkWidget3->setMaximumSize(2000,2000);
  mitkWidget4->setMaximumSize(2000,2000);

  layout3 ->addWidget( mitkWidget3,0,0 );
  layout3 ->addWidget( mitkWidget4,1,0 );

  layout4->addLayout( layout3 );
  layout4->addWidget( levelWindowWidget );
  QmitkStdMultiWidgetLayout->addLayout( layout4 );
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
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget1->GetRenderer()))->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget2->GetRenderer()))->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget3->GetRenderer()))->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget4->GetRenderer()))->GetVtkRenderer();
  if ( vtkrenderer!=NULL ) vtkrenderer->ResetCamera();
  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::AddDisplayPlaneSubTree(mitk::DataTreeIteratorBase* it)
{
  // add the diplayed planes of the multiwidget to a node to which the subtree @a planesSubTree points ...

  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  mitk::DataTreeIteratorClone dit = it;
  dit->Add(node);
  dit->GoToChild(dit->ChildPosition(node));
  planesSubTree=dynamic_cast<mitk::DataTreeBase*>(dit->GetSubTree());

  float white[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  // ... of widget 1
  planeNode=mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitkWidget4->GetRenderer());
  planeNode->SetProperty("name", new mitk::StringProperty("widget1Plane"));
  planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
  dit->Add(planeNode);
  // ... of widget 2
  planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitkWidget4->GetRenderer());
  planeNode->SetProperty("name", new mitk::StringProperty("widget2Plane"));
  planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
  dit->Add(planeNode);
  // ... of widget 3
  planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(white, mitkWidget4->GetRenderer());
  planeNode->SetProperty("name", new mitk::StringProperty("widget3Plane"));
  planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
  dit->Add(planeNode);

  planesIterator = dit;
}

mitk::MultiplexUpdateController* QmitkStdMultiWidget::GetMultiplexUpdateController()
{
  return multiplexUpdateController.GetPointer();
}


mitk::SliceNavigationController* QmitkStdMultiWidget::GetTimeNavigationController()
{
  return timeNavigationController.GetPointer();
}

void QmitkStdMultiWidget::EnableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  connect(levelWindowWidget,SIGNAL(levelWindow(mitk::LevelWindow*)),this,SLOT(changeLevelWindow(mitk::LevelWindow*)) );
  levelWindowWidget->show();
}

void QmitkStdMultiWidget::DisableStandardLevelWindow()
{
  levelWindowWidget->disconnect(this);
  levelWindowWidget->hide();
}

bool QmitkStdMultiWidget::InitializeStandardViews(mitk::DataTreeIteratorBase * it)
{
  bool boundingBoxInitialized = false;

  mitk::SliceNavigationController* sliceNavigatorTransversal = mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal    = mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal     = mitkWidget3->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorWidget4     = mitkWidget4->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);
  sliceNavigatorWidget4->SetViewDirection(mitk::SliceNavigationController::Original);

  if ( it==NULL )
  {
    multiplexUpdateController->SetBlockUpdate(true);
    sliceNavigatorTransversal->Update();
    sliceNavigatorSagittal->Update();
    sliceNavigatorFrontal->Update();
    timeNavigationController->Update();
    multiplexUpdateController->SetBlockUpdate(false);
    multiplexUpdateController->UpdateRequest();
  }
  else
  {
    const mitk::BoundingBox::Pointer boundingbox = mitk::DataTree::ComputeVisibleBoundingBox(it, NULL, "includeInBoundingBox");
    if ( boundingbox->GetPoints()->Size()>0 )
    {
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
      geometry->Initialize();
      geometry->SetBounds(boundingbox->GetBounds());

      //lets see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = mitk::DataTree::ComputeTimeBounds(it, NULL, "includeInBoundingBox");
      if ( timebounds[1]<mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry = mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(geometry, (unsigned int) duration);
        timegeometry->SetTimeBounds(timebounds); //@bug really required? FIXME

        timebounds[1] = timebounds[0]+1.0f;
        geometry->SetTimeBounds(timebounds);

        geometry=timegeometry;
      }

      double diagonalLength  = const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2();
      if ( (diagonalLength > mitk::eps) && (diagonalLength < mitk::large) )
      {
        boundingBoxInitialized=true;
        
        multiplexUpdateController->SetBlockUpdate(true);
        
        sliceNavigatorTransversal->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorTransversal->Update();

        sliceNavigatorSagittal->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorSagittal->Update();

        sliceNavigatorFrontal->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorFrontal->Update();

        sliceNavigatorWidget4->SetInputWorldGeometry(geometry.GetPointer());
        sliceNavigatorWidget4->Update();

        timeNavigationController->SetInputWorldGeometry(geometry.GetPointer());
        timeNavigationController->Update();

        multiplexUpdateController->SetBlockUpdate(false);

        // Temporary solution: Use RepaintRequest instead of UpdateRequest so
        // that Fit() resets the camera according to the new geometry.
        //multiplexUpdateController->UpdateRequest();
        multiplexUpdateController->RepaintRequest();
        Fit();
      }
    }
  }
  return boundingBoxInitialized;
}


bool QmitkStdMultiWidget::InitializeStandardViews( mitk::Geometry3D * geometry )
{
  bool boundingBoxInitialized = false;

  mitk::SliceNavigationController* sliceNavigatorTransversal = mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal    = mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal     = mitkWidget3->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);

  if ( (geometry==NULL) || (const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2()<mitk::eps) )
  {
    multiplexUpdateController->SetBlockUpdate(true);
    sliceNavigatorTransversal->Update();
    sliceNavigatorSagittal->Update();
    sliceNavigatorFrontal->Update();
    timeNavigationController->Update();
    multiplexUpdateController->SetBlockUpdate(false);
    multiplexUpdateController->UpdateRequest();
  }
  else
  {
    mitk::Geometry3D::Pointer clonedgeometry = static_cast<mitk::Geometry3D*>(geometry->Clone().GetPointer());

    multiplexUpdateController->SetBlockUpdate(true);
    sliceNavigatorTransversal->SetInputWorldGeometry(clonedgeometry.GetPointer()); sliceNavigatorTransversal->Update();
    sliceNavigatorSagittal->SetInputWorldGeometry(clonedgeometry.GetPointer());    sliceNavigatorSagittal->Update();
    sliceNavigatorFrontal->SetInputWorldGeometry(clonedgeometry.GetPointer());     sliceNavigatorFrontal->Update();
    timeNavigationController->SetInputWorldGeometry(clonedgeometry.GetPointer());  timeNavigationController->Update();
    multiplexUpdateController->SetBlockUpdate(false);
    multiplexUpdateController->UpdateRequest();
    Fit();
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
  return m_MoveAndZoomInteractor;
}

void QmitkStdMultiWidget::ReInitializeStandardViews()
{
  mitk::SliceNavigationController* sliceNavigatorTransversal = mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal    = mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal     = mitkWidget3->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);

  multiplexUpdateController->SetBlockUpdate(true);
  sliceNavigatorTransversal->Update();
  sliceNavigatorSagittal->Update();
  sliceNavigatorFrontal->Update();
  timeNavigationController->Update();
  multiplexUpdateController->SetBlockUpdate(false);
  multiplexUpdateController->UpdateRequest();
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

void QmitkStdMultiWidget::changeLevelWindow(mitk::LevelWindow* lw)
{
  mitk::DataTreeIteratorClone it = mitkWidget1->GetRenderer()->GetData();
  int maxLayer = itk::NumericTraits<int>::min();
  mitk::LevelWindowProperty::Pointer topLevWinProp = NULL;
  while ( !it->IsAtEnd() )
  {
    if ( (it->Get().IsNotNull()) && (it->Get()->IsVisible(NULL)) )
    {
      int layer = 0;
      it->Get()->GetIntProperty("layer", layer);
      if ( layer >= maxLayer )
      {
        bool binary = false;
        it->Get()->GetBoolProperty("binary", binary);
        if( binary == false)
        {
          mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(it->Get()->GetProperty("levelwindow").GetPointer());
          if (levWinProp.IsNotNull())
          {
            topLevWinProp = levWinProp;
            maxLayer = layer;
          }
        }
      }
    }
    ++it;
  }
  if ( topLevWinProp.IsNotNull() )
  {
    mitk::LevelWindow levWin = topLevWinProp->GetLevelWindow();

    levWin.SetMin(lw->GetMin());
    levWin.SetMax(lw->GetMax());
    levWin.SetRangeMin(lw->GetRangeMin());
    levWin.SetRangeMax(lw->GetRangeMax());

    topLevWinProp->SetLevelWindow(levWin);
    this->RequestUpdate();
  }
} // changeLevelWindow()

const mitk::Point3D & QmitkStdMultiWidget::GetCrossPosition() const
{
  return m_LastLeftClickPositionSupplier->GetCurrentPoint();
}

void QmitkStdMultiWidget::AdjustCross()
{
  mitk::Point2D p2d;
  mitk::Point3D p3d;
  p3d = GetCrossPosition();
  mitk::PositionEvent event(mitkWidget1->GetRenderer(), 0, 0, 0, mitk::Key_unknown, p2d, p3d);
  mitk::StateEvent *stateEvent = new mitk::StateEvent(mitk::EIDLEFTMOUSEBTN , &event);    
  mitk::GlobalInteraction* globalInteraction = mitk::GlobalInteraction::GetGlobalInteraction();
  if(globalInteraction!=NULL)
  {
    globalInteraction->HandleEvent( stateEvent );
    stateEvent->Set(mitk::EIDLEFTMOUSERELEASE , &event);
    globalInteraction->HandleEvent( stateEvent );
  }
  delete stateEvent;  
}


