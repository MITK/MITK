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

#include "mitkSurfaceDeformationDataInteractor3D.h"


#include "mitkMouseWheelEvent.h"

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkInteractorObserver.h>

mitk::SurfaceDeformationDataInteractor3D::SurfaceDeformationDataInteractor3D()
  :m_GaussSigma(30.0)
{
  m_OriginalPolyData = vtkPolyData::New();

  // Initialize vector arithmetic
  m_ObjectNormal[0] = 0.0;
  m_ObjectNormal[1] = 0.0;
  m_ObjectNormal[2] = 1.0;
}

mitk::SurfaceDeformationDataInteractor3D::~SurfaceDeformationDataInteractor3D()
{
  m_OriginalPolyData->Delete();
}

void mitk::SurfaceDeformationDataInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before
  // actually executing an action
  CONNECT_CONDITION("isOverObject", CheckOverObject);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject",SelectObject);
  CONNECT_FUNCTION("deselectObject",DeselectObject);
  CONNECT_FUNCTION("initDeformation",InitDeformation);
  CONNECT_FUNCTION("deformObject",DeformObject);
  CONNECT_FUNCTION("scaleRadius", ScaleRadius);
}

void mitk::SurfaceDeformationDataInteractor3D::DataNodeChanged()
{
  if(this->GetDataNode().IsNotNull())
  {
    m_Surface = dynamic_cast<Surface*>(this->GetDataNode()->GetData());

    if (m_Surface == NULL)
      MITK_ERROR << "SurfaceDeformationDataInteractor3D::DataNodeChanged(): DataNode has to contain a surface.";
  }
  else
    m_Surface = NULL;
}

bool mitk::SurfaceDeformationDataInteractor3D::CheckOverObject(const InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  Point2D currentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  Point3D currentPickedPoint;

  if(interactionEvent->GetSender()->PickObject(currentPickedDisplayPoint, currentPickedPoint) == this->GetDataNode().GetPointer())
  {
    // Colorized surface at current picked position
    m_SurfaceColorizationCenter = currentPickedPoint;
    return true;
  }
  return false;
}

bool mitk::SurfaceDeformationDataInteractor3D::SelectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  vtkPolyData* polyData = m_Surface->GetVtkPolyData(timeStep);

  this->GetDataNode()->SetColor(1.0, 0.0, 0.0);

  // Colorize surface / wireframe dependend on distance from picked point
  this->ColorizeSurface(polyData, timeStep,  m_SurfaceColorizationCenter, COLORIZATION_GAUSS);

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
  return true;
}

bool mitk::SurfaceDeformationDataInteractor3D::DeselectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  vtkPolyData* polyData = m_Surface->GetVtkPolyData(timeStep);

  this->GetDataNode()->SetColor(1.0, 1.0, 1.0);

  // Colorize surface / wireframe as inactive
  this->ColorizeSurface(polyData, timeStep, m_SurfaceColorizationCenter, COLORIZATION_CONSTANT, -1.0);

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
  return true;
}

bool mitk::SurfaceDeformationDataInteractor3D::InitDeformation(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  vtkPolyData* polyData = m_Surface->GetVtkPolyData(timeStep);

  // Store current picked point
  mitk::Point2D currentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  interactionEvent->GetSender()->PickObject(currentPickedDisplayPoint, m_InitialPickedPoint);

  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    currentPickedDisplayPoint[0],
    currentPickedDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    m_InitialPickedWorldPoint);

  // Make deep copy of vtkPolyData interacted on
  m_OriginalPolyData->DeepCopy(polyData);

  return true;
}

bool mitk::SurfaceDeformationDataInteractor3D::DeformObject (StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  vtkPolyData* polyData = m_Surface->GetVtkPolyData(timeStep);
  BaseGeometry::Pointer geometry = this->GetDataNode()->GetData()->GetGeometry(timeStep);

  double currentWorldPoint[4];
  mitk::Point2D currentDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  vtkInteractorObserver::ComputeDisplayToWorld(
    interactionEvent->GetSender()->GetVtkRenderer(),
    currentDisplayPoint[0],
    currentDisplayPoint[1],
    0.0, //m_InitialInteractionPickedPoint[2],
    currentWorldPoint);

  // Calculate mouse move in 3D space
  Vector3D interactionMove;
  interactionMove[0] = currentWorldPoint[0] - m_InitialPickedWorldPoint[0];
  interactionMove[1] = currentWorldPoint[1] - m_InitialPickedWorldPoint[1];
  interactionMove[2] = currentWorldPoint[2] - m_InitialPickedWorldPoint[2];

  // Transform mouse move into geometry space
  this->GetDataNode()->GetData()->UpdateOutputInformation();// make sure that the Geometry is up-to-date
  Vector3D interactionMoveIndex;
  geometry->WorldToIndex(interactionMove, interactionMoveIndex);

  // Get picked point and transform into local coordinates
  Point3D pickedPoint;
  geometry->WorldToIndex(m_InitialPickedPoint, pickedPoint);

  Vector3D v1 = pickedPoint.GetVectorFromOrigin();

  vtkDataArray* normal = polyData->GetPointData()->GetVectors("planeNormal");
  if (normal != NULL)
  {
    m_ObjectNormal[0] = normal->GetComponent(0, 0);
    m_ObjectNormal[1] = normal->GetComponent(0, 1);
    m_ObjectNormal[2] = normal->GetComponent(0, 2);
  }

  Vector3D v2 = m_ObjectNormal * (interactionMoveIndex * m_ObjectNormal);

  vtkPoints* originalPoints = m_OriginalPolyData->GetPoints();
  vtkPoints* deformedPoints = polyData->GetPoints();

  double denom = m_GaussSigma * m_GaussSigma * 2;
  double point[3];
  for (vtkIdType i = 0; i < deformedPoints->GetNumberOfPoints(); ++i)
  {
    // Get original point
    double* originalPoint = originalPoints->GetPoint( i );

    Vector3D v0;
    v0[0] = originalPoint[0];
    v0[1] = originalPoint[1];
    v0[2] = originalPoint[2];

    // Calculate distance of this point from line through picked point
    double d = itk::CrossProduct(m_ObjectNormal, (v1 - v0)).GetNorm();

    Vector3D t = v2 * exp(- d * d / denom);

    point[0] = originalPoint[0] + t[0];
    point[1] = originalPoint[1] + t[1];
    point[2] = originalPoint[2] + t[2];
    deformedPoints->SetPoint(i, point);
  }

  // Make sure that surface is colorized at initial picked position as long as we are in deformation state
  m_SurfaceColorizationCenter = m_InitialPickedPoint;

  polyData->Modified();
  m_Surface->Modified();

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
  return true;
}

bool mitk::SurfaceDeformationDataInteractor3D::ScaleRadius(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const MouseWheelEvent* wheelEvent = dynamic_cast<const MouseWheelEvent*>(interactionEvent);
  if(wheelEvent == NULL)
    return false;

  m_GaussSigma += (double) (wheelEvent->GetWheelDelta()) / 20;
  if ( m_GaussSigma < 10.0 )
  {
    m_GaussSigma = 10.0;
  }
  else if ( m_GaussSigma > 128.0 )
  {
    m_GaussSigma = 128.0;
  }

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  vtkPolyData* polyData = m_Surface->GetVtkPolyData(timeStep);

  // Colorize surface / wireframe dependend on sigma and distance from picked point
  this->ColorizeSurface( polyData, timeStep, m_SurfaceColorizationCenter, COLORIZATION_GAUSS );

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
  return true;
}


bool mitk::SurfaceDeformationDataInteractor3D::ColorizeSurface(vtkPolyData* polyData, int timeStep, const Point3D &pickedPoint, int mode, double scalar)
{
  if (polyData == NULL)
    return false;

  vtkPoints* points = polyData->GetPoints();
  vtkPointData* pointData = polyData->GetPointData();
  if ( pointData == NULL )
    return false;

  vtkDataArray* scalars = pointData->GetScalars();
  if (scalars == NULL)
    return false;

  if (mode == COLORIZATION_GAUSS)
  {
    // Get picked point and transform into local coordinates
    Point3D localPickedPoint;
    BaseGeometry::Pointer geometry = this->GetDataNode()->GetData()->GetGeometry(timeStep);
    geometry->WorldToIndex( pickedPoint, localPickedPoint );

    Vector3D v1 = localPickedPoint.GetVectorFromOrigin();

    vtkDataArray* normal = polyData->GetPointData()->GetVectors("planeNormal");
    if (normal != NULL)
    {
      m_ObjectNormal[0] = normal->GetComponent(0, 0);
      m_ObjectNormal[1] = normal->GetComponent(0, 1);
      m_ObjectNormal[2] = normal->GetComponent(0, 2);
    }

    double denom = m_GaussSigma * m_GaussSigma * 2;
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
      // Get original point
      double* point = points->GetPoint(i);
      Vector3D v0;
      v0[0] = point[0];
      v0[1] = point[1];
      v0[2] = point[2];

      // Calculate distance of this point from line through picked point
      double d = itk::CrossProduct(m_ObjectNormal, (v1 - v0)).GetNorm();
      double t = exp(- d * d / denom);

      scalars->SetComponent(i, 0, t);
    }
  }
  else if (mode == COLORIZATION_CONSTANT)
  {
    for (vtkIdType i = 0; i < pointData->GetNumberOfTuples(); ++i)
    {
      scalars->SetComponent(i, 0, scalar);
    }
  }

  polyData->Modified();
  pointData->Update();

  return true;
}
