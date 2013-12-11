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

#include "mitkDispatcher.h"
#include "mitkInteractionConst.h" // TODO: refactor file
#include "mitkInteractionPositionEvent.h"
#include "mitkInternalEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkRenderingManager.h"
#include "mitkRotationOperation.h"
#include "mitkSurface.h"

#include <mitkPointOperation.h>

#include <vtkCamera.h>
#include <vtkDataArray.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

mitk::SurfaceDeformationDataInteractor3D::SurfaceDeformationDataInteractor3D()
  :m_Precision( 6.5 ),
  m_GaussSigma( 30.0 )
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

void mitk::SurfaceDeformationDataInteractor3D::SetPrecision( mitk::ScalarType precision )
{
  m_Precision = precision;
}

void mitk::SurfaceDeformationDataInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine,
  // to ensure that certain conditions are met, before
  // actually executing an action
  CONNECT_CONDITION("isOverObject", CheckOverObject);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject",SelectObject);
  CONNECT_FUNCTION("deselectObject",DeselectObject);
  CONNECT_FUNCTION("initDeformation",InitDeformation);
  CONNECT_FUNCTION("deformObject",DeformObject);

}

void mitk::SurfaceDeformationDataInteractor3D::DataNodeChanged()
{
  if(this->GetDataNode() != NULL)
  {
    m_Surface = dynamic_cast< Surface * >(this->GetDataNode()->GetData());

    if ( m_Surface == NULL )
    {
      MITK_ERROR << "SurfaceDeformationDataInteractor3D::DataNodeChanged(): DataNode has to contain a surface.";
    }
  }
  else
  {
    m_Surface = NULL;
  }
}

bool mitk::SurfaceDeformationDataInteractor3D::CheckOverObject(const InteractionEvent* interactionEvent)
{
  ////Is only a copy of the old SurfaceDeformationInteractor3D. Not sure if is still needed.
  ////Re-enable VTK interactor (may have been disabled previously)
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_CurrentPickedPoint = positionEvent->GetPositionInWorld();
  m_CurrentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  if(interactionEvent->GetSender()->PickObject( m_CurrentPickedDisplayPoint, m_CurrentPickedPoint ) == this->GetDataNode().GetPointer())
  {
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

  mitk::Point3D surfaceColorizationCenter = positionEvent->GetPositionInWorld();

  this->GetDataNode()->SetColor( 1.0, 0.0, 0.0 );
  //TODO: Only 3D reinit
  RenderingManager::GetInstance()->RequestUpdateAll();

  // Colorize surface / wireframe dependend on distance from picked point
  //TODO Check the return value
  this->ColorizeSurface( interactionEvent->GetSender(), m_Surface->GetVtkPolyData(timeStep),
    surfaceColorizationCenter, COLORIZATION_GAUSS );

  return true;
}

bool mitk::SurfaceDeformationDataInteractor3D::DeselectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  mitk::Point3D surfaceColorizationCenter = positionEvent->GetPositionInWorld();

  this->GetDataNode()->SetColor( 1.0, 1.0, 1.0 );
  //TODO: Only 3D reinit
  RenderingManager::GetInstance()->RequestUpdateAll();

  // Colorize surface / wireframe as inactive
  //TODO Check the return value
  this->ColorizeSurface( interactionEvent->GetSender(), m_Surface->GetVtkPolyData(timeStep),
    surfaceColorizationCenter, COLORIZATION_CONSTANT, -1.0 );

  return true;
}

bool mitk::SurfaceDeformationDataInteractor3D::InitDeformation(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  // Store current picked point
  m_InitialPickedPoint = positionEvent->GetPositionInWorld();

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of vtkPolyData interacted on
  m_OriginalPolyData->DeepCopy(m_Surface->GetVtkPolyData(timeStep));
}

bool mitk::SurfaceDeformationDataInteractor3D::DeformObject (StateMachineAction*, InteractionEvent* interactionEvent)
{

  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  // Calculate mouse move in 3D space
  mitk::Vector3D interactionMove;
  interactionMove[0] = m_CurrentPickedPoint[0] - m_InitialPickedPoint[0];
  interactionMove[1] = m_CurrentPickedPoint[1] - m_InitialPickedPoint[1];
  interactionMove[2] = m_CurrentPickedPoint[2] - m_InitialPickedPoint[2];

  this->GetDataNode()->GetData()->UpdateOutputInformation();

  mitk::Point3D origin; origin.Fill( 0.0 );
  mitk::Vector3D interactionMoveIndex;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  mitk::Geometry3D::Pointer geometry = this->GetDataNode()->GetData()->GetGeometry(timeStep);
  geometry->WorldToIndex(interactionMove, interactionMoveIndex);

   // Get picked point and transform into local coordinates
      mitk::Point3D pickedPoint;
      geometry->WorldToIndex(m_InitialPickedPoint, pickedPoint);
      mitk::Vector3D v1 = pickedPoint.GetVectorFromOrigin();

      mitk::Vector3D v2 = m_ObjectNormal * (interactionMoveIndex * m_ObjectNormal);

      vtkPoints* originalPoints = m_OriginalPolyData->GetPoints();
      vtkPoints* deformedPoints = m_Surface->GetVtkPolyData(timeStep)->GetPoints();
       double denom = m_GaussSigma * m_GaussSigma * 2;
      double point[3];
      for ( unsigned int i = 0; i < deformedPoints->GetNumberOfPoints(); ++i )
      {
        // Get original point
        double *originalPoint = originalPoints->GetPoint( i );
        mitk::Vector3D v0;
        v0[0] = originalPoint[0];
        v0[1] = originalPoint[1];
        v0[2] = originalPoint[2];


        // Calculate distance of this point from line through picked point
        double d = itk::CrossProduct( m_ObjectNormal, (v1 - v0) ).GetNorm();

        mitk::Vector3D t = v2 * exp( - d * d / denom );

        point[0] = originalPoint[0] + t[0];
        point[1] = originalPoint[1] + t[1];
        point[2] = originalPoint[2] + t[2];
        deformedPoints->SetPoint( i, point );
      }

      // TOODODOODODODODODODOODODOODODODODODOODODODODODO

      m_PolyData->Modified();
      m_Surface->Modified();

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}


bool mitk::SurfaceDeformationDataInteractor3D::ColorizeSurface(BaseRenderer::Pointer renderer, double scalar)
{
  BaseData::Pointer data = this->GetDataNode()->GetData();
  if(data.IsNull())
  {
    MITK_ERROR << "SurfaceDeformationInteractor3D: No data object present!";
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
    MITK_ERROR << "SurfaceDeformationInteractor3D: No poly data present!";
    return false;
  }

  vtkPointData *pointData = polyData->GetPointData();
  if (pointData == NULL)
  {
    MITK_ERROR << "SurfaceDeformationInteractor3D: No point data present!";
    return false;
  }

  vtkDataArray *scalars = pointData->GetScalars();
  if (scalars == NULL)
  {
    MITK_ERROR << "SurfaceDeformationInteractor3D: No scalars for point data present!";
    return false;
  }

  for (unsigned int i = 0; i < pointData->GetNumberOfTuples(); ++i)
  {
    scalars->SetComponent(i, 0, scalar);
  }

  polyData->Modified();
  pointData->Update();

  return true;
}
