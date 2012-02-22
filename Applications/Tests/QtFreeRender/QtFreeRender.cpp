/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 21985 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkRenderWindow.h"

#include <mitkDataNodeFactory.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkProperties.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingManager.h>
#include <mitkGlobalInteraction.h>
#include "mitkProperties.h"
#include "mitkGeometry2DDataMapper2D.h"
#include "mitkGlobalInteraction.h"
#include "mitkDisplayInteractor.h"
#include "mitkPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkLine.h"
#include "mitkInteractionConst.h"
#include "mitkVtkLayerController.h"
#include "mitkPositionTracker.h"
#include "mitkDisplayVectorInteractor.h"
#include "mitkSlicesRotator.h"
#include "mitkSlicesSwiveller.h"
#include "mitkRenderWindowFrame.h"
#include "mitkGradientBackground.h"
#include "mitkCoordinateSupplier.h"
#include "mitkDataStorage.h"


#include "vtkTextProperty.h"
#include "vtkCornerAnnotation.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAnnotatedCubeActor.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkProperty.h"

//##Documentation
//## @brief Example of a NON QT DEPENDENT MITK RENDERING APPLICATION. 

mitk::DisplayVectorInteractor::Pointer    m_MoveAndZoomInteractor;
mitk::CoordinateSupplier::Pointer         m_LastLeftClickPositionSupplier;
mitk::GradientBackground::Pointer         m_GradientBackground4;
mitk::RenderWindowFrame::Pointer          m_RectangleRendering1;
mitk::RenderWindowFrame::Pointer          m_RectangleRendering2;
mitk::RenderWindowFrame::Pointer          m_RectangleRendering3;
mitk::RenderWindowFrame::Pointer          m_RectangleRendering4;

mitk::SliceNavigationController::Pointer m_TimeNavigationController;

mitk::DataStorage::Pointer m_DataStorage;
mitk::DataNode::Pointer m_PlaneNode1;
mitk::DataNode::Pointer m_PlaneNode2;
mitk::DataNode::Pointer m_PlaneNode3;
mitk::DataNode::Pointer m_Node;

mitk::RenderWindow::Pointer mitkWidget1;
mitk::RenderWindow::Pointer mitkWidget2;
mitk::RenderWindow::Pointer mitkWidget3;
mitk::RenderWindow::Pointer mitkWidget4;

void InitializeWindows()
{
  
  // Set default view directions for SNCs
  mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Transversal );
  mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Sagittal );
  mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Frontal );
  mitkWidget4->GetSliceNavigationController()->SetDefaultViewDirection(
    mitk::SliceNavigationController::Original );

    //initialize m_TimeNavigationController: send time via sliceNavigationControllers
  m_TimeNavigationController = mitk::SliceNavigationController::New("dummy");
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget1->GetSliceNavigationController() , false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget2->GetSliceNavigationController() , false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget3->GetSliceNavigationController() , false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(
    mitkWidget4->GetSliceNavigationController() , false);
  mitkWidget1->GetSliceNavigationController()
    ->ConnectGeometrySendEvent(mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));

  // Set TimeNavigationController to RenderingManager
  // (which uses it internally for views initialization!)
  mitk::RenderingManager::GetInstance()->SetTimeNavigationController(
    m_TimeNavigationController );

  //reverse connection between sliceNavigationControllers and m_TimeNavigationController
  mitkWidget1->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController.GetPointer(), false);
  mitkWidget2->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController.GetPointer(), false);
  mitkWidget3->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController.GetPointer(), false);
  mitkWidget4->GetSliceNavigationController()
    ->ConnectGeometryTimeEvent(m_TimeNavigationController.GetPointer(), false);


  // Let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();
  gi->AddListener( m_TimeNavigationController );
    
  m_LastLeftClickPositionSupplier =
    mitk::CoordinateSupplier::New("navigation", NULL);
  mitk::GlobalInteraction::GetInstance()->AddListener(
    m_LastLeftClickPositionSupplier
    );
 
  m_GradientBackground4 = mitk::GradientBackground::New();
  m_GradientBackground4->SetRenderWindow(
    mitkWidget4->GetVtkRenderWindow() );
  m_GradientBackground4->SetGradientColors(0.1,0.1,0.1,0.5,0.5,0.5);
  m_GradientBackground4->Enable();

  m_RectangleRendering1 = mitk::RenderWindowFrame::New();
  m_RectangleRendering1->SetRenderWindow(
    mitkWidget1->GetVtkRenderWindow() );
  m_RectangleRendering1->Enable(1.0,0.0,0.0);

  m_RectangleRendering2 = mitk::RenderWindowFrame::New();
  m_RectangleRendering2->SetRenderWindow(
    mitkWidget2->GetVtkRenderWindow() );
  m_RectangleRendering2->Enable(0.0,1.0,0.0);

  m_RectangleRendering3 = mitk::RenderWindowFrame::New();
  m_RectangleRendering3->SetRenderWindow(
    mitkWidget3->GetVtkRenderWindow() );
  m_RectangleRendering3->Enable(0.0,0.0,1.0);

  m_RectangleRendering4 = mitk::RenderWindowFrame::New();
  m_RectangleRendering4->SetRenderWindow(
    mitkWidget4->GetVtkRenderWindow() );
  m_RectangleRendering4->Enable(1.0,1.0,0.0);

}


void AddDisplayPlaneSubTree()
{
  // add the displayed planes of the multiwidget to a node to which the subtree 
  // @a planesSubTree points ...

  float white[3] = {1.0f,1.0f,1.0f};
  mitk::Geometry2DDataMapper2D::Pointer mapper;
  mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1000);

  // ... of widget 1
  m_PlaneNode1 = (mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow()))->GetCurrentWorldGeometry2DNode();
  m_PlaneNode1->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode1->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode1->SetProperty("name", mitk::StringProperty::New("widget1Plane"));
  m_PlaneNode1->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode1->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode1->SetProperty("layer",layer);
  m_PlaneNode1->SetColor(1.0,0.0,0.0);
  mapper = mitk::Geometry2DDataMapper2D::New();
  m_PlaneNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 2
  m_PlaneNode2 =( mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow()))->GetCurrentWorldGeometry2DNode();
  m_PlaneNode2->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode2->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("name", mitk::StringProperty::New("widget2Plane"));
  m_PlaneNode2->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode2->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("layer",layer);
  m_PlaneNode2->SetColor(0.0,1.0,0.0);
  mapper = mitk::Geometry2DDataMapper2D::New();
  m_PlaneNode2->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 3
  m_PlaneNode3 = (mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow()))->GetCurrentWorldGeometry2DNode();
  m_PlaneNode3->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("name", mitk::StringProperty::New("widget3Plane"));
  m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("layer",layer);
  m_PlaneNode3->SetColor(0.0,0.0,1.0);
  mapper = mitk::Geometry2DDataMapper2D::New();
  m_PlaneNode3->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  m_Node = mitk::DataNode::New();
  m_Node->SetProperty("name", mitk::StringProperty::New("Widgets"));
  m_Node->SetProperty("helper object", mitk::BoolProperty::New(true));

  //AddPlanesToDataStorage
  if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull() && m_PlaneNode3.IsNotNull() && m_Node.IsNotNull())
  {
    if (m_DataStorage.IsNotNull())
    {
      m_DataStorage->Add(m_Node);
      m_DataStorage->Add(m_PlaneNode1, m_Node);
      m_DataStorage->Add(m_PlaneNode2, m_Node);
      m_DataStorage->Add(m_PlaneNode3, m_Node);
      static_cast<mitk::Geometry2DDataMapper2D*>(m_PlaneNode1->GetMapper(mitk::BaseRenderer::Standard2D))->SetDatastorageAndGeometryBaseNode(m_DataStorage, m_Node);
      static_cast<mitk::Geometry2DDataMapper2D*>(m_PlaneNode2->GetMapper(mitk::BaseRenderer::Standard2D))->SetDatastorageAndGeometryBaseNode(m_DataStorage, m_Node);
      static_cast<mitk::Geometry2DDataMapper2D*>(m_PlaneNode3->GetMapper(mitk::BaseRenderer::Standard2D))->SetDatastorageAndGeometryBaseNode(m_DataStorage, m_Node);
    }
  }
}


void Fit()
{
  vtkRenderer * vtkrenderer;
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow())->GetDisplayGeometry()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow())->GetDisplayGeometry()->Fit();

  int w = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL ) 
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL ) 
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL ) 
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow())->GetVtkRenderer();
  if ( vtkrenderer!= NULL ) 
    vtkrenderer->ResetCamera();

  vtkObject::SetGlobalWarningDisplay(w);
}

int main(int argc, char* argv[])
{
  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", "" );
    return 1;
  }

  // Create a DataStorage  
  m_DataStorage = mitk::StandaloneDataStorage::New();

  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for(i=1; i<argc; ++i)
  {
    // For testing
    if(strcmp(argv[i], "-testing")==0) continue;

    // Create a DataNodeFactory to read a data format supported
    // by the DataNodeFactory (many image formats, surface formats, etc.)
    mitk::DataNodeFactory::Pointer nodeReader=mitk::DataNodeFactory::New();
    const char * filename = argv[i];
    try
    {
      nodeReader->SetFileName(filename);
      nodeReader->Update();

      // Since the DataNodeFactory directly creates a node,
      // use the datastorage to add the read node
      mitk::DataNode::Pointer node = nodeReader->GetOutput();
      m_DataStorage->Add(node);

      
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
      if(image.IsNotNull())
      {
        // Set the property "volumerendering" to the Boolean value "true"
        node->SetProperty("volumerendering", mitk::BoolProperty::New(false));
        node->SetProperty("name", mitk::StringProperty::New("testimage"));
        node->SetProperty("layer",mitk::IntProperty::New(1));
      }
    }
    catch(...)
    {
      fprintf( stderr, "Could not open file %s \n\n", filename );
      exit(2);
    }
  }

  //*************************************************************************
  // Part V: Create window and pass the tree to it
  //*************************************************************************

  // Global Interaction initialize 
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // instantiate display interactor
  m_MoveAndZoomInteractor = mitk::DisplayVectorInteractor::New(
    "moveNzoom", new mitk::DisplayInteractor() );

  mitk::GlobalInteraction::GetInstance()->AddListener(m_MoveAndZoomInteractor);

  
  // Create renderwindows
  mitkWidget1 = mitk::RenderWindow::New();
  mitkWidget2 = mitk::RenderWindow::New();
  mitkWidget3 = mitk::RenderWindow::New();
  mitkWidget4 = mitk::RenderWindow::New();
 
  // Tell the renderwindow which (part of) the datastorage to render
  mitkWidget1->GetRenderer()->SetDataStorage(m_DataStorage);
  mitkWidget2->GetRenderer()->SetDataStorage(m_DataStorage);
  mitkWidget3->GetRenderer()->SetDataStorage(m_DataStorage);
  mitkWidget4->GetRenderer()->SetDataStorage(m_DataStorage);

 
  // Let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();
  gi->AddListener( mitkWidget1->GetSliceNavigationController() );
  gi->AddListener( mitkWidget2->GetSliceNavigationController() );
  gi->AddListener( mitkWidget3->GetSliceNavigationController() );
  gi->AddListener( mitkWidget4->GetSliceNavigationController() );
  
  // Use it as a 2D View
  mitkWidget1->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  mitkWidget2->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  mitkWidget3->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  mitkWidget4->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
 

  mitkWidget1->SetSize(400, 400);
  
  
  mitkWidget2->GetVtkRenderWindow()->SetPosition(mitkWidget1->GetVtkRenderWindow()->GetPosition()[0]+420,
                                                 mitkWidget1->GetVtkRenderWindow()->GetPosition()[1]);
  mitkWidget2->SetSize(400, 400);
  
  
  mitkWidget3->GetVtkRenderWindow()->SetPosition(mitkWidget1->GetVtkRenderWindow()->GetPosition()[0],
                                                 mitkWidget1->GetVtkRenderWindow()->GetPosition()[1]+450);
  mitkWidget3->SetSize(400, 400);
  
  
  mitkWidget4->GetVtkRenderWindow()->SetPosition(mitkWidget1->GetVtkRenderWindow()->GetPosition()[0]+420,
                                                 mitkWidget1->GetVtkRenderWindow()->GetPosition()[1]+450);
  mitkWidget4->SetSize(400, 400);


  InitializeWindows();
  
  AddDisplayPlaneSubTree();
 
  Fit();

  // Initialize the RenderWindows
  mitk::TimeSlicedGeometry::Pointer geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );


  m_DataStorage->Print( std::cout );
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

  // reinit the mitkVTKEventProvider;
  // this is only necessary once after calling 
  // ForceImmediateUpdateAll() for the first time
  mitkWidget1->ReinitEventProvider();
  mitkWidget2->ReinitEventProvider();
  mitkWidget3->ReinitEventProvider();


  mitkWidget1->GetVtkRenderWindow()->Render();
  mitkWidget2->GetVtkRenderWindow()->Render();
  mitkWidget3->GetVtkRenderWindow()->Render();
  mitkWidget4->GetVtkRenderWindow()->Render();
  mitkWidget4->GetVtkRenderWindowInteractor()->Start();

 
  return 0;
}

