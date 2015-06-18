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

#include "mitkRenderWindow.h"

#include <mitkStandaloneDataStorage.h>
#include <mitkProperties.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingManager.h>
#include <mitkGlobalInteraction.h>
#include "mitkProperties.h"
#include "mitkPlaneGeometryDataMapper2D.h"
#include "mitkGlobalInteraction.h"
#include "mitkDisplayInteractor.h"
#include "mitkPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkLine.h"
#include "mitkInteractionConst.h"
#include "mitkVtkLayerController.h"
#include "mitkPositionTracker.h"
#include "mitkDisplayInteractor.h"
#include "mitkSlicesRotator.h"
#include "mitkSlicesSwiveller.h"
#include "mitkCoordinateSupplier.h"
#include "mitkDataStorage.h"
#include "mitkIOUtil.h"

#include "vtkTextProperty.h"
#include "vtkCornerAnnotation.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAnnotatedCubeActor.h"
#include "vtkMitkRectangleProp.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkProperty.h"

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"

#include "mitkInteractionEventObserver.h"

//##Documentation
//## @brief Example of a NON QT DEPENDENT MITK RENDERING APPLICATION.

mitk::RenderWindow::Pointer mitkWidget1;
mitk::RenderWindow::Pointer mitkWidget2;
mitk::RenderWindow::Pointer mitkWidget3;
mitk::RenderWindow::Pointer mitkWidget4;

mitk::DisplayInteractor::Pointer m_DisplayInteractor;
mitk::CoordinateSupplier::Pointer m_LastLeftClickPositionSupplier;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering1;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering2;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering3;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering4;

mitk::SliceNavigationController* m_TimeNavigationController = NULL;

mitk::DataStorage::Pointer m_DataStorage;
mitk::DataNode::Pointer m_PlaneNode1;
mitk::DataNode::Pointer m_PlaneNode2;
mitk::DataNode::Pointer m_PlaneNode3;
mitk::DataNode::Pointer m_Node;

void InitializeWindows()
{

  // Set default view directions for SNCs
  mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Axial);
  mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
  mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Frontal);
  mitkWidget4->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Original);

  //initialize m_TimeNavigationController: send time via sliceNavigationControllers
  m_TimeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget1->GetSliceNavigationController(), false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget2->GetSliceNavigationController(), false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget3->GetSliceNavigationController(), false);
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget4->GetSliceNavigationController(), false);
  mitkWidget1->GetSliceNavigationController()->ConnectGeometrySendEvent(mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));

  //reverse connection between sliceNavigationControllers and m_TimeNavigationController
  mitkWidget1->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  mitkWidget2->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  mitkWidget3->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
  mitkWidget4->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);

  // Let NavigationControllers listen to GlobalInteraction
  mitk::GlobalInteraction *gi = mitk::GlobalInteraction::GetInstance();
  gi->AddListener(m_TimeNavigationController);

  m_LastLeftClickPositionSupplier = mitk::CoordinateSupplier::New("navigation", NULL);
  mitk::GlobalInteraction::GetInstance()->AddListener(m_LastLeftClickPositionSupplier);

  mitkWidget4->GetRenderer()->GetVtkRenderer()->SetBackground(0.1,0.1,0.1);
  mitkWidget4->GetRenderer()->GetVtkRenderer()->SetBackground(0.5,0.5,0.5);
  mitkWidget4->GetRenderer()->GetVtkRenderer()->GradientBackgroundOn();

  m_RectangleRendering1 = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleRendering1->SetColor(1.0, 0.0, 0.0);
  mitkWidget1->GetRenderer()->GetVtkRenderer()->AddViewProp(m_RectangleRendering1);

  m_RectangleRendering2 = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleRendering2->SetColor(0.0, 1.0, 0.0);
  mitkWidget2->GetRenderer()->GetVtkRenderer()->AddViewProp(m_RectangleRendering2);

  m_RectangleRendering3 = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleRendering3->SetColor(0.0, 0.0, 1.0);
  mitkWidget3->GetRenderer()->GetVtkRenderer()->AddViewProp(m_RectangleRendering3);

  m_RectangleRendering4 = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleRendering4->SetColor(1.0, 1.0, 0.0);
  mitkWidget4->GetRenderer()->GetVtkRenderer()->AddViewProp(m_RectangleRendering4);

}

void AddDisplayPlaneSubTree()
{
  // add the displayed planes of the multiwidget to a node to which the subtree
  // @a planesSubTree points ...

  float white[3] =
  { 1.0f, 1.0f, 1.0f };
  mitk::PlaneGeometryDataMapper2D::Pointer mapper;
  mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1000);

  // ... of widget 1
  m_PlaneNode1 = (mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode1->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode1->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode1->SetProperty("name", mitk::StringProperty::New("widget1Plane"));
  m_PlaneNode1->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode1->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode1->SetProperty("layer", layer);
  m_PlaneNode1->SetColor(1.0, 0.0, 0.0);
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 2
  m_PlaneNode2 = (mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode2->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode2->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("name", mitk::StringProperty::New("widget2Plane"));
  m_PlaneNode2->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode2->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode2->SetProperty("layer", layer);
  m_PlaneNode2->SetColor(0.0, 1.0, 0.0);
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode2->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // ... of widget 3
  m_PlaneNode3 = (mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("name", mitk::StringProperty::New("widget3Plane"));
  m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("layer", layer);
  m_PlaneNode3->SetColor(0.0, 0.0, 1.0);
  mapper = mitk::PlaneGeometryDataMapper2D::New();
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
  if (vtkrenderer != NULL)
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != NULL)
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != NULL)
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != NULL)
    vtkrenderer->ResetCamera();

  vtkObject::SetGlobalWarningDisplay(w);
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage:   %s [filename1] [filename2] ...\n\n", "");
    return 1;
  }

  // Create a DataStorage
  m_DataStorage = mitk::StandaloneDataStorage::New();

  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for (i = 1; i < argc; ++i)
  {
    // For testing
    if (strcmp(argv[i], "-testing") == 0)
      continue;

    std::string filename = argv[i];
    try
    {
      // Read the file and add it as a data node to the data storage
      mitk::DataStorage::SetOfObjects::Pointer nodes = mitk::IOUtil::Load(filename, *m_DataStorage);

      for (mitk::DataStorage::SetOfObjects::Iterator nodeIter = nodes->Begin(),
           nodeIterEnd = nodes->End(); nodeIter != nodeIterEnd; ++nodeIter)
      {
        mitk::DataNode::Pointer node = nodeIter->Value();
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
        if (image.IsNotNull())
        {
          // Set the property "volumerendering" to the Boolean value "true"
          node->SetProperty("volumerendering", mitk::BoolProperty::New(false));
          node->SetProperty("name", mitk::StringProperty::New("testimage"));
          node->SetProperty("layer", mitk::IntProperty::New(1));
        }
      }
    } catch (...)
    {
      std::cerr << "Could not open file " << filename << std::endl;
      exit(2);
    }
  }

  //*************************************************************************
  // Part V: Create window and pass the tree to it
  //*************************************************************************

  // Global Interaction initialize
  // legacy because window manager relies still on existence if global interaction
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  //mitk::GlobalInteraction::GetInstance()->AddListener(m_DisplayInteractor);

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
  gi->AddListener(mitkWidget1->GetSliceNavigationController());
  gi->AddListener(mitkWidget2->GetSliceNavigationController());
  gi->AddListener(mitkWidget3->GetSliceNavigationController());
  gi->AddListener(mitkWidget4->GetSliceNavigationController());

  // instantiate display interactor
  if (m_DisplayInteractor.IsNull())
  {
    m_DisplayInteractor = mitk::DisplayInteractor::New();
    m_DisplayInteractor->LoadStateMachine("DisplayInteraction.xml");
    m_DisplayInteractor->SetEventConfig("DisplayConfigMITK.xml");
    // Register as listener via micro services

    us::ModuleContext* context = us::GetModuleContext();
    context->RegisterService<mitk::InteractionEventObserver>(
        m_DisplayInteractor.GetPointer());
  }
  // Use it as a 2D View
  mitkWidget1->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  mitkWidget2->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  mitkWidget3->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  mitkWidget4->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

  mitkWidget1->SetSize(400, 400);

  mitkWidget2->GetVtkRenderWindow()->SetPosition(mitkWidget1->GetVtkRenderWindow()->GetPosition()[0] + 420,
      mitkWidget1->GetVtkRenderWindow()->GetPosition()[1]);
  mitkWidget2->SetSize(400, 400);

  mitkWidget3->GetVtkRenderWindow()->SetPosition(mitkWidget1->GetVtkRenderWindow()->GetPosition()[0],
      mitkWidget1->GetVtkRenderWindow()->GetPosition()[1] + 450);
  mitkWidget3->SetSize(400, 400);

  mitkWidget4->GetVtkRenderWindow()->SetPosition(mitkWidget1->GetVtkRenderWindow()->GetPosition()[0] + 420,
      mitkWidget1->GetVtkRenderWindow()->GetPosition()[1] + 450);
  mitkWidget4->SetSize(400, 400);

  InitializeWindows();

  AddDisplayPlaneSubTree();

  Fit();

  // Initialize the RenderWindows
  mitk::TimeGeometry::Pointer geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  m_DataStorage->Print(std::cout);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

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

