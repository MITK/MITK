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
#include <mitkGeometry2DDataVtkMapper3D.h>
#include <vtkRenderer.h>
#include <mitkOpenGLRenderer.h>

void QmitkStdMultiWidget::updateMitkWidgets()
{
  mitkWidget1->GetRenderer()->GetRenderWindow()->Repaint(); 
  mitkWidget2->GetRenderer()->GetRenderWindow()->Repaint(); 
  mitkWidget3->GetRenderer()->GetRenderWindow()->Repaint(); 
  mitkWidget4->GetRenderer()->GetRenderWindow()->Repaint();
}

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
  layer = new mitk::IntProperty(1);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 2
  qcolor=mitkWidget2->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 3
  qcolor=mitkWidget3->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 4
  qcolor=mitkWidget4->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget4->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  
  //initialize timeNavigationController
  timeNavigationController = new mitk::SliceNavigationController(NULL);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget1->GetRenderer(), false);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget2->GetRenderer(), false);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget3->GetRenderer(), false);
  timeNavigationController->ConnectGeometryTimeEvent(mitkWidget4->GetRenderer(), false);
  mitkWidget1->GetSliceNavigationController()->ConnectGeometrySendEvent(mitkWidget4->GetRenderer());
  
  //initialize update-controller
  multiplexUpdateController = new mitk::MultiplexUpdateController("navigation");
  multiplexUpdateController->AddRenderWindow(mitkWidget1->GetRenderer()->GetRenderWindow());
  multiplexUpdateController->AddRenderWindow(mitkWidget2->GetRenderer()->GetRenderWindow());
  multiplexUpdateController->AddRenderWindow(mitkWidget3->GetRenderer()->GetRenderWindow());
  multiplexUpdateController->AddRenderWindow(mitkWidget4->GetRenderer()->GetRenderWindow());
  
  //connect sliceNavigationControllers to update-controller
  mitkWidget1->GetSliceNavigationController()->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
  mitkWidget2->GetSliceNavigationController()->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
  mitkWidget3->GetSliceNavigationController()->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
  mitkWidget4->GetSliceNavigationController()->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
  timeNavigationController->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
  
  // instantiate display interactor
  m_MoveAndZoomInteractor = new mitk::DisplayVectorInteractor("moveNzoom", new mitk::DisplayInteractor() );
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
   
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   if (mitkWidget2->isHidden()) mitkWidget2->show();
   if (mitkWidget3->isHidden()) mitkWidget3->show();
   if (mitkWidget4->isHidden()) mitkWidget4->show();
      
      
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
   
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   if (mitkWidget2->isHidden()) mitkWidget2->show();
   if (mitkWidget3->isHidden()) mitkWidget3->show();
   if (mitkWidget4->isHidden()) mitkWidget4->show();
   
   
   layout3->addLayout( TwoDLayout , 0, 0 );
   layout3->addWidget( mitkWidget4, 0,1 );
    
    
    layout4->addLayout( layout3 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}
    
void QmitkStdMultiWidget::changeLayoutToDefault()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to defualt... " << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    QGridLayout *layout3 = new QGridLayout( 0, 2, 2, 0, 6, "layout3");

   
    mitkWidget1->setMaximumSize(2000,2000);
    mitkWidget2->setMaximumSize(2000,2000);
    mitkWidget3->setMaximumSize(2000,2000);    
    
   layout3 ->addWidget( mitkWidget1,0,0 );
   layout3 ->addWidget( mitkWidget2,0,1 );
   layout3 ->addWidget( mitkWidget3,1,0 );
   
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   if (mitkWidget2->isHidden()) mitkWidget2->show();
   if (mitkWidget3->isHidden()) mitkWidget3->show();
   if (mitkWidget4->isHidden()) mitkWidget4->show();
   
   layout3->addWidget( mitkWidget4, 1,1 );
    
    
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
   if (mitkWidget4->isHidden()) mitkWidget4->show();
   
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
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   
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
   if (mitkWidget2->isHidden()) mitkWidget2->show();    
   
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
  if (mitkWidget3->isHidden()) mitkWidget3->show();
  
    layout4->addWidget( mitkWidget3 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}

void QmitkStdMultiWidget::setData( mitk::DataTreeIteratorBase* it )
{
  mitkWidget1->GetRenderer()->SetData(it);
  mitkWidget2->GetRenderer()->SetData(it);
  mitkWidget3->GetRenderer()->SetData(it);
  mitkWidget4->GetRenderer()->SetData(it);
}

void QmitkStdMultiWidget::fit()
{
  vtkRenderer * vtkrenderer;
  mitkWidget1->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget2->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget3->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget4->GetRenderer()->GetDisplayGeometry()->Fit();

  int w=vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget1->GetRenderer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget2->GetRenderer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget3->GetRenderer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget4->GetRenderer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::initWidget(mitk::DataTreeIteratorBase* it,
                                   QmitkSelectableGLWidget* widget,
		        const mitk::Vector3D& origin,
                                   const mitk::Vector3D& right,
                                   const mitk::Vector3D& bottom)
{

}

void QmitkStdMultiWidget::initWidgets( mitk::DataTreeIteratorBase* it )
{

}

void QmitkStdMultiWidget::addPlaneSubTree(mitk::DataTreeIteratorBase* it)
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

void QmitkStdMultiWidget::texturizePlaneSubTree(mitk::DataTreeIteratorBase* it)
{
  if(planesSubTree == NULL)
      return;

  assert(planesIterator.IsNotNull());
  mitk::DataTreeIteratorClone git = planesIterator; //planesSubTree->inorderIterator();

  while(!git->IsAtEnd())
  {
    if(dynamic_cast<mitk::Geometry2DData*>(git->Get()->GetData())!=NULL)
    {
      mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper;
      if(git->Get()->GetMapper(2)==NULL)
      {
        geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
        it->Get()->SetMapper(2, geometryMapper);
      }
      else
        geometryMapper = dynamic_cast<mitk::Geometry2DDataVtkMapper3D*>(git->Get()->GetMapper(2));
      if(geometryMapper.IsNotNull())
        geometryMapper->SetDataIteratorForTexture(it);
    }
    ++git;
  }
}


mitk::MultiplexUpdateController* QmitkStdMultiWidget::GetMultiplexUpdateController()
{
    return multiplexUpdateController.GetPointer();
}


mitk::SliceNavigationController* QmitkStdMultiWidget::GetTimeNavigationController()
{
    return timeNavigationController.GetPointer();
}


bool QmitkStdMultiWidget::InitializeStandardViews(mitk::DataTreeIteratorBase * it)
{
  bool boundingBoxInitialized = false;

  mitk::SliceNavigationController* sliceNavigatorTransversal = mitkWidget1->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorSagittal       = mitkWidget2->GetSliceNavigationController();
  mitk::SliceNavigationController* sliceNavigatorFrontal        = mitkWidget3->GetSliceNavigationController();

  sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
  sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);

  if(it==NULL)
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
    if(boundingbox->GetPoints()->Size()>0)
    { 
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
      geometry->Initialize();
      geometry->SetBounds(boundingbox->GetBounds());

      //lets see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = mitk::DataTree::ComputeTimeBoundsInMS(it, NULL, "includeInBoundingBox");
      if(timebounds[1]<mitk::ScalarTypeNumericTraits::max())
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry = mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(geometry, (unsigned int) duration);
        timegeometry->SetTimeBoundsInMS(timebounds); //@bug really required? FIXME

        timebounds[1] = timebounds[0]+1.0f;
        geometry->SetTimeBoundsInMS(timebounds);

        geometry=timegeometry;
      }

      if(const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2()>=mitk::eps)
      {
        boundingBoxInitialized=true;
        multiplexUpdateController->SetBlockUpdate(true);
        sliceNavigatorTransversal->SetInputWorldGeometry(geometry.GetPointer()); sliceNavigatorTransversal->Update();
        sliceNavigatorSagittal->SetInputWorldGeometry(geometry.GetPointer());    sliceNavigatorSagittal->Update();
        sliceNavigatorFrontal->SetInputWorldGeometry(geometry.GetPointer());     sliceNavigatorFrontal->Update();
        timeNavigationController->SetInputWorldGeometry(geometry.GetPointer());  timeNavigationController->Update();
        multiplexUpdateController->SetBlockUpdate(false);
        multiplexUpdateController->UpdateRequest();
        fit();
      }
    }
  }
  return boundingBoxInitialized;
}


//bool QmitkStdMultiWidget::InitializeStandardViews(mitk::SlicedGeometry* geometry)
//{
//  bool boundingBoxInitialized = false;
//
//  mitk::SliceNavigationController* sliceNavigatorTransversal = mitkWidget1->GetSliceNavigationController();
//  mitk::SliceNavigationController* sliceNavigatorSagittal       = mitkWidget2->GetSliceNavigationController();
//  mitk::SliceNavigationController* sliceNavigatorFrontal        = mitkWidget3->GetSliceNavigationController();
//
//  sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
//  sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
//  sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);
//
//  if(geometry==NULL)
//  {
//    multiplexUpdateController->SetBlockUpdate(true);
//    sliceNavigatorTransversal->Update();
//    sliceNavigatorSagittal->Update();
//    sliceNavigatorFrontal->Update();
//    timeNavigationController->Update();
//    multiplexUpdateController->SetBlockUpdate(false);
//    multiplexUpdateController->UpdateRequest();
//  }
//  else
//  {
//    const mitk::BoundingBox* boundingbox = geometry->GetBoundingBox();
//    if(boundingbox->GetPoints()->Size()>0)
//    { 
//      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
//      geometry->Initialize();
//      geometry->SetBounds(boundingbox->GetBounds());
//
//      //lets see if we have data with a limited live-span ...
//      mitk::TimeBounds timebounds = mitk::DataTree::ComputeTimeBoundsInMS(it, NULL, "includeInBoundingBox");
//      if(timebounds[1]<mitk::ScalarTypeNumericTraits::max())
//      {
//        mitk::ScalarType duration = timebounds[1]-timebounds[0];
//
//        mitk::TimeSlicedGeometry::Pointer timegeometry = mitk::TimeSlicedGeometry::New();
//        timegeometry->InitializeEvenlyTimed(geometry, (unsigned int) duration);
//        timegeometry->SetTimeBoundsInMS(timebounds); //@bug really required? FIXME
//
//        timebounds[1] = timebounds[0]+1.0f;
//        geometry->SetTimeBoundsInMS(timebounds);
//
//        geometry=timegeometry;
//      }
//
//      if(const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2()>=mitk::eps)
//      {
//        boundingBoxInitialized=true;	
//        multiplexUpdateController->SetBlockUpdate(true);
//        sliceNavigatorTransversal->SetInputWorldGeometry(geometry.GetPointer()); sliceNavigatorTransversal->Update();
//        sliceNavigatorSagittal->SetInputWorldGeometry(geometry.GetPointer());    sliceNavigatorSagittal->Update();
//        sliceNavigatorFrontal->SetInputWorldGeometry(geometry.GetPointer());     sliceNavigatorFrontal->Update();
//        timeNavigationController->SetInputWorldGeometry(geometry.GetPointer());  timeNavigationController->Update();
//        multiplexUpdateController->SetBlockUpdate(false);
//        multiplexUpdateController->UpdateRequest();
//        fit();
//      }
//    }
//  }
//  return boundingBoxInitialized;
//}

void QmitkStdMultiWidget::wheelEvent( QWheelEvent * e )
{
    std::cout << "QmitkMultiWidget: grabbed mouse wheel event" << std::endl;
    emit WheelMoved( e );
}


mitk::DisplayVectorInteractor* QmitkStdMultiWidget::GetMoveAndZoomInteractor()
{
    return m_MoveAndZoomInteractor;
}
