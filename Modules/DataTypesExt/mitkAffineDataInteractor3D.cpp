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

#include "mitkAffineDataInteractor3D.h"

#include "mitkInteractionConst.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkRotationOperation.h"
#include "mitkSurface.h"

#include <vtkCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>


mitk::AffineDataInteractor3D::AffineDataInteractor3D()
{
  m_OriginalGeometry = Geometry3D::New();

  // Initialize vector arithmetic
  m_ObjectNormal[0] = 0.0;
  m_ObjectNormal[1] = 0.0;
  m_ObjectNormal[2] = 1.0;
}

mitk::AffineDataInteractor3D::~AffineDataInteractor3D()
{
}

void mitk::AffineDataInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually executing an action
  CONNECT_CONDITION("isOverObject", CheckOverObject);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject",SelectObject);
  CONNECT_FUNCTION("deselectObject",DeselectObject);
  CONNECT_FUNCTION("initTranslate",InitTranslate);
  CONNECT_FUNCTION("initRotate",InitRotate);
  CONNECT_FUNCTION("translateObject",TranslateObject);
  CONNECT_FUNCTION("rotateObject",RotateObject);
}

void mitk::AffineDataInteractor3D::DataNodeChanged()
{
}

bool mitk::AffineDataInteractor3D::CheckOverObject(const InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  Point3D currentWorldPoint;
  if(interactionEvent->GetSender()->PickObject(positionEvent->GetPointerPositionOnScreen(), currentWorldPoint) == this->GetDataNode().GetPointer())
    return true;

  return false;
}

bool mitk::AffineDataInteractor3D::SelectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return false;

  node->SetColor(1.0, 0.0, 0.0);

  // Colorize surface / wireframe dependend on distance from picked point
  this->ColorizeSurface(interactionEvent->GetSender(), 0.0);

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::AffineDataInteractor3D::DeselectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return false;

  node->SetColor( 1.0, 1.0, 1.0 );

  // Colorize surface / wireframe as inactive
  this->ColorizeSurface(interactionEvent->GetSender(), -1.0);

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::AffineDataInteractor3D::InitTranslate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    m_InitialPickedDisplayPoint[0],
    m_InitialPickedDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    m_InitialPickedWorldPoint );

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if ((interactionEvent->GetSender()) != NULL)
    timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry = static_cast<Geometry3D*>(this->GetDataNode()->GetData()->GetGeometry(timeStep)->Clone().GetPointer());

  return true;
}

bool mitk::AffineDataInteractor3D::InitRotate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    m_InitialPickedDisplayPoint[0],
    m_InitialPickedDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    m_InitialPickedWorldPoint );

  // Get the timestep to also support 3D+t
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry = static_cast<Geometry3D*>(this->GetDataNode()->GetData()->GetGeometry(timeStep)->Clone().GetPointer());

  return true;
}

bool mitk::AffineDataInteractor3D::TranslateObject (StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  double currentWorldPoint[4];
  mitk::Point2D currentDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    currentDisplayPoint[0],
    currentDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    currentWorldPoint);

  Vector3D interactionMove;
  interactionMove[0] = currentWorldPoint[0] - m_InitialPickedWorldPoint[0];
  interactionMove[1] = currentWorldPoint[1] - m_InitialPickedWorldPoint[1];
  interactionMove[2] = currentWorldPoint[2] - m_InitialPickedWorldPoint[2];

  Point3D origin = m_OriginalGeometry->GetOrigin();

  // Get the timestep to also support 3D+t
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // If data is an mitk::Surface, extract it
  Surface::Pointer surface = dynamic_cast< Surface* >(this->GetDataNode()->GetData());
  vtkPolyData* polyData = NULL;
  if (surface.IsNotNull())
  {
    polyData = surface->GetVtkPolyData( timeStep );

    // Extract surface normal from surface (if existent, otherwise use default)
    vtkPointData* pointData = polyData->GetPointData();
    if (pointData != NULL)
    {
      vtkDataArray* normal = polyData->GetPointData()->GetVectors("planeNormal");
      if (normal != NULL)
      {
        m_ObjectNormal[0] = normal->GetComponent( 0, 0 );
        m_ObjectNormal[1] = normal->GetComponent( 0, 1 );
        m_ObjectNormal[2] = normal->GetComponent( 0, 2 );
      }
    }
  }

  Vector3D transformedObjectNormal;
  this->GetDataNode()->GetData()->GetGeometry( timeStep )->IndexToWorld(m_ObjectNormal, transformedObjectNormal);

  this->GetDataNode()->GetData()->GetGeometry( timeStep )->SetOrigin(origin + transformedObjectNormal * (interactionMove * transformedObjectNormal));

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  return true;
}

bool mitk::AffineDataInteractor3D::RotateObject (StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  double currentWorldPoint[4];
  Point2D currentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    currentPickedDisplayPoint[0],
    currentPickedDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    currentWorldPoint);

  vtkCamera* camera = NULL;
  vtkRenderer* currentVtkRenderer = NULL;

  if ((interactionEvent->GetSender()) != NULL)
  {
    vtkRenderWindow* renderWindow = interactionEvent->GetSender()->GetRenderWindow();
    if (renderWindow != NULL)
    {
      vtkRenderWindowInteractor* renderWindowInteractor = renderWindow->GetInteractor();
      if ( renderWindowInteractor != NULL )
      {
        currentVtkRenderer = renderWindowInteractor->GetInteractorStyle()->GetCurrentRenderer();
        if (currentVtkRenderer != NULL)
          camera = currentVtkRenderer->GetActiveCamera();
      }
    }
  }
  if ( camera )
  {
    double vpn[3];
    camera->GetViewPlaneNormal( vpn );

    Vector3D viewPlaneNormal;
    viewPlaneNormal[0] = vpn[0];
    viewPlaneNormal[1] = vpn[1];
    viewPlaneNormal[2] = vpn[2];

    Vector3D interactionMove;
    interactionMove[0] = currentWorldPoint[0] - m_InitialPickedWorldPoint[0];
    interactionMove[1] = currentWorldPoint[1] - m_InitialPickedWorldPoint[1];
    interactionMove[2] = currentWorldPoint[2] - m_InitialPickedWorldPoint[2];

    if (interactionMove[0] == 0 && interactionMove[1] == 0  && interactionMove[2] == 0)
      return true;

    Vector3D rotationAxis = itk::CrossProduct(viewPlaneNormal, interactionMove);
    rotationAxis.Normalize();

    int* size = currentVtkRenderer->GetSize();
    double l2 =
      (currentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) *
      (currentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) +
      (currentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]) *
      (currentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]);

    double rotationAngle = 360.0 * sqrt(l2 / (size[0] * size[0] + size[1] * size[1]));

    // Use center of data bounding box as center of rotation
    Point3D rotationCenter = m_OriginalGeometry->GetCenter();

    int timeStep = 0;
    if ((interactionEvent->GetSender()) != NULL)
      timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

    // Reset current Geometry3D to original state (pre-interaction) and
    // apply rotation
    RotationOperation op( OpROTATE, rotationCenter, rotationAxis, rotationAngle );
    Geometry3D::Pointer newGeometry = static_cast<Geometry3D*>(m_OriginalGeometry->Clone().GetPointer());
    newGeometry->ExecuteOperation( &op );
    mitk::TimeGeometry::Pointer timeGeometry = this->GetDataNode()->GetData()->GetTimeGeometry();
    if (timeGeometry.IsNotNull())
      timeGeometry->SetTimeStepGeometry(newGeometry, timeStep);

    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

    return true;
  }
  else
    return false;
}


bool mitk::AffineDataInteractor3D::ColorizeSurface(BaseRenderer::Pointer renderer, double scalar)
{
  BaseData::Pointer data = this->GetDataNode()->GetData();
  if(data.IsNull())
  {
    MITK_ERROR << "AffineInteractor3D: No data object present!";
    return false;
  }

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if (renderer.IsNotNull())
    timeStep = renderer->GetTimeStep(data);


  // If data is an mitk::Surface, extract it
  Surface::Pointer surface = dynamic_cast< Surface* >(data.GetPointer());
  vtkPolyData* polyData = NULL;
  if (surface.IsNotNull())
    polyData = surface->GetVtkPolyData(timeStep);

  if (polyData == NULL)
  {
    MITK_ERROR << "AffineInteractor3D: No poly data present!";
    return false;
  }

  vtkPointData* pointData = polyData->GetPointData();
  if (pointData == NULL)
  {
    MITK_ERROR << "AffineInteractor3D: No point data present!";
    return false;
  }

  vtkDataArray* scalars = pointData->GetScalars();
  if (scalars == NULL)
  {
    MITK_ERROR << "AffineInteractor3D: No scalars for point data present!";
    return false;
  }

  for (vtkIdType i = 0; i < pointData->GetNumberOfTuples(); ++i)
  {
    scalars->SetComponent(i, 0, scalar);
  }

  polyData->Modified();
  pointData->Update();

  return true;
}
