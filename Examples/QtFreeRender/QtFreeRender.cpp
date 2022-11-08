/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderWindow.h"

#include <mitkCameraController.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkProperties.h>
#include <mitkVtkLayerController.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>

#include <mitkDataStorage.h>
#include <mitkIOUtil.h>

#include <vtkAnnotatedCubeActor.h>
#include <vtkCornerAnnotation.h>
#include <vtkMitkRectangleProp.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextProperty.h>

//##Documentation
//## @brief Example of a NON QT DEPENDENT MITK RENDERING APPLICATION.

mitk::RenderWindow::Pointer mitkWidget1;
mitk::RenderWindow::Pointer mitkWidget2;
mitk::RenderWindow::Pointer mitkWidget3;
mitk::RenderWindow::Pointer mitkWidget4;

mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering1;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering2;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering3;
vtkSmartPointer<vtkMitkRectangleProp> m_RectangleRendering4;

mitk::SliceNavigationController *m_TimeNavigationController = nullptr;

mitk::DataStorage::Pointer m_DataStorage;
mitk::DataNode::Pointer m_PlaneNode1;
mitk::DataNode::Pointer m_PlaneNode2;
mitk::DataNode::Pointer m_PlaneNode3;
mitk::DataNode::Pointer m_Node;

void InitializeWindows()
{
  // Set default view directions for SNCs
  mitkWidget1->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);
  mitkWidget2->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Sagittal);
  mitkWidget3->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Coronal);
  mitkWidget4->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Original);

  // initialize m_TimeNavigationController: send time via sliceNavigationControllers
  m_TimeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget1->GetSliceNavigationController());
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget2->GetSliceNavigationController());
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget3->GetSliceNavigationController());
  m_TimeNavigationController->ConnectGeometryTimeEvent(mitkWidget4->GetSliceNavigationController());
  mitkWidget1->GetSliceNavigationController()->ConnectGeometrySendEvent(
    mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));

  // reverse connection between sliceNavigationControllers and m_TimeNavigationController
  mitkWidget1->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController);
  mitkWidget2->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController);
  mitkWidget3->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController);
  mitkWidget4->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController);

  mitkWidget4->GetRenderer()->GetVtkRenderer()->SetBackground(0.1, 0.1, 0.1);
  mitkWidget4->GetRenderer()->GetVtkRenderer()->SetBackground(0.5, 0.5, 0.5);
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

  float white[3] = {1.0f, 1.0f, 1.0f};
  mitk::PlaneGeometryDataMapper2D::Pointer mapper;
  mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1000);

  // ... of widget 1
  m_PlaneNode1 =
    (mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();
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
  m_PlaneNode2 =
    (mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();
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
  m_PlaneNode3 =
    (mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();
  m_PlaneNode3->SetColor(white, mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow()));
  m_PlaneNode3->SetProperty("visible", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("name", mitk::StringProperty::New("widget3Plane"));
  m_PlaneNode3->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  m_PlaneNode3->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PlaneNode3->SetProperty("layer", layer);
  m_PlaneNode3->SetColor(0.0, 0.0, 1.0);
  mapper = mitk::PlaneGeometryDataMapper2D::New();
  m_PlaneNode3->SetMapper(mitk::BaseRenderer::Standard2D, mapper);

  // AddPlanesToDataStorage
  if (m_PlaneNode1.IsNotNull() && m_PlaneNode2.IsNotNull() && m_PlaneNode3.IsNotNull() && m_Node.IsNotNull())
  {
    if (m_DataStorage.IsNotNull())
    {
      m_DataStorage->Add(m_PlaneNode1);
      m_DataStorage->Add(m_PlaneNode2);
      m_DataStorage->Add(m_PlaneNode3);
    }
  }
}

void Fit()
{
  vtkRenderer *vtkrenderer;
  mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow())->GetCameraController()->Fit();
  mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow())->GetCameraController()->Fit();

  int w = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget1->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != nullptr)
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget2->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != nullptr)
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget3->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != nullptr)
    vtkrenderer->ResetCamera();

  vtkrenderer = mitk::BaseRenderer::GetInstance(mitkWidget4->GetVtkRenderWindow())->GetVtkRenderer();
  if (vtkrenderer != nullptr)
    vtkrenderer->ResetCamera();

  vtkObject::SetGlobalWarningDisplay(w);
}

int main(int argc, char *argv[])
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

      for (mitk::DataStorage::SetOfObjects::Iterator nodeIter = nodes->Begin(), nodeIterEnd = nodes->End();
           nodeIter != nodeIterEnd;
           ++nodeIter)
      {
        mitk::DataNode::Pointer node = nodeIter->Value();
        mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());
        if (image.IsNotNull())
        {
          // Set the property "volumerendering" to the Boolean value "true"
          node->SetProperty("volumerendering", mitk::BoolProperty::New(false));
          node->SetProperty("name", mitk::StringProperty::New("testimage"));
          node->SetProperty("layer", mitk::IntProperty::New(1));
        }
      }
    }
    catch (...)
    {
      std::cerr << "Could not open file " << filename << std::endl;
      exit(2);
    }
  }

  //*************************************************************************
  // Part V: Create window and pass the tree to it
  //*************************************************************************

  // Create renderwindows
  mitkWidget1 = mitk::RenderWindow::New();
  mitkWidget2 = mitk::RenderWindow::New();
  mitkWidget3 = mitk::RenderWindow::New();
  mitkWidget4 = mitk::RenderWindow::New();

  mitkWidget1->GetRenderer()->PrepareRender();
  mitkWidget2->GetRenderer()->PrepareRender();
  mitkWidget3->GetRenderer()->PrepareRender();

  // Tell the renderwindow which (part of) the datastorage to render
  mitkWidget1->GetRenderer()->SetDataStorage(m_DataStorage);
  mitkWidget2->GetRenderer()->SetDataStorage(m_DataStorage);
  mitkWidget3->GetRenderer()->SetDataStorage(m_DataStorage);
  mitkWidget4->GetRenderer()->SetDataStorage(m_DataStorage);

  // instantiate display interactor
  if (m_DisplayActionEventBroadcast.IsNull())
  {
    m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
    m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
    m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigMITK.xml");
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
  auto geo = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
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
