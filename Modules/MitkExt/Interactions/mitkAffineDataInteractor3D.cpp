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


#include "mitkDispatcher.h"
#include "mitkInteractionConst.h" // TODO: refactor file
#include "mitkInteractionPositionEvent.h"
#include "mitkInternalEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkRenderingManager.h"
#include "mitkSurface.h"

#include <mitkPointOperation.h>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

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
  CONNECT_FUNCTION("checkOverObject",CheckOverObject);
  CONNECT_FUNCTION("selectObject",SelectObject);
  CONNECT_FUNCTION("deselectObject",DeselectObject);
  CONNECT_FUNCTION("initTranslate",InitTranslate);
  CONNECT_FUNCTION("initRotate",InitRotate);
  CONNECT_FUNCTION("translateObject",TranslateObject);
  CONNECT_FUNCTION("rotateObject",RotateObject);
  //CONNECT_FUNCTION("endTranslate",EndTranslate);
  //CONNECT_FUNCTION("endRotate",EndRotate);
}

/*
* Check whether the DataNode contains a pointset, if not create one and add it.
*/
void mitk::AffineDataInteractor3D::DataNodeChanged()
{
  //if (GetDataNode().IsNotNull())
  //{
  //  // find proper place for this command!
  //  // maybe when DN is created ?
  //  GetDataNode()->SetBoolProperty("show contour", true);
  //  PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
  //  if (points == NULL)
  //  {
  //    m_PointSet = PointSet::New();
  //    GetDataNode()->SetData(m_PointSet);
  //  }
  //  else
  //  {
  //    m_PointSet = points;
  //  }
  //  // load config file parameter: maximal number of points
  //  mitk::PropertyList::Pointer properties = GetAttributes();
  //  std::string strNumber;
  //  if (properties->GetStringProperty("MaxPoints", strNumber))
  //  {
  //    m_MaxNumberOfPoints = atoi(strNumber.c_str());
  //  }
  //}
}

bool mitk::AffineDataInteractor3D::CheckOverObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  ////Is only a copy of the old AffineInteractor3D. Not sure if is still needed.
  ////Re-enable VTK interactor (may have been disabled previously)
  //if ( renderWindowInteractor != NULL )
  //  renderWindowInteractor->Enable();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_CurrentPickedPoint = positionEvent->GetPositionInWorld();
  m_CurrentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  if(interactionEvent->GetSender()->PickObject( m_CurrentPickedDisplayPoint, m_CurrentPickedPoint ) == this->GetDataNode().GetPointer())
  {
    //Object will be selected
    InternalEvent::Pointer event = InternalEvent::New(NULL, this, "OverObject");
    positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
  }

  return true;
}

bool mitk::AffineDataInteractor3D::SelectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return false;

  node->SetColor( 1.0, 0.0, 0.0 );
  //TODO: Only 3D reinit
  RenderingManager::GetInstance()->RequestUpdateAll();

  // Colorize surface / wireframe dependend on distance from picked point
  //TODO Check the return value
  this->ColorizeSurface( interactionEvent->GetSender(), 0.0 );

  return true;
}

bool mitk::AffineDataInteractor3D::DeselectObject(StateMachineAction*, InteractionEvent* interactionEvent)
{
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return false;

  node->SetColor( 1.0, 1.0, 1.0 );
  //TODO: Only 3D reinit
  RenderingManager::GetInstance()->RequestUpdateAll();

  // Colorize surface / wireframe as inactive
  //TODO Check the return value
  this->ColorizeSurface( interactionEvent->GetSender(), -1.0 );

  return true;
}

bool mitk::AffineDataInteractor3D::InitTranslate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  ////Is only a copy of the old AffineInteractor3D. Not sure if is still needed.
  //// Disable VTK interactor until MITK interaction has been completed
  //    if ( renderWindowInteractor != NULL )
  //      renderWindowInteractor->Disable();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_CurrentPickedPoint = positionEvent->GetPositionInWorld();
  m_CurrentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  m_InitialPickedPoint = m_CurrentPickedPoint;
  m_InitialPickedDisplayPoint = m_CurrentPickedDisplayPoint;

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if ((interactionEvent->GetSender()) != NULL)
    timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry = static_cast< Geometry3D * >(this->GetDataNode()->GetData()->GetGeometry( timeStep )->Clone().GetPointer() );

  return true;
}

bool mitk::AffineDataInteractor3D::InitRotate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  ////Is only a copy of the old AffineInteractor3D. Not sure if is still needed.
  //// Disable VTK interactor until MITK interaction has been completed
  //    if ( renderWindowInteractor != NULL )
  //      renderWindowInteractor->Disable();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_CurrentPickedPoint = positionEvent->GetPositionInWorld();
  m_CurrentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  m_InitialPickedPoint = m_CurrentPickedPoint;
  m_InitialPickedDisplayPoint = m_CurrentPickedDisplayPoint;

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if ((interactionEvent->GetSender()) != NULL)
    timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

  // Make deep copy of current Geometry3D of the plane
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
  m_OriginalGeometry = static_cast< Geometry3D * >(this->GetDataNode()->GetData()->GetGeometry( timeStep )->Clone().GetPointer() );

  return true;
}

bool mitk::AffineDataInteractor3D::TranslateObject (StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  m_CurrentPickedPoint = positionEvent->GetPositionInWorld();
  m_CurrentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  Vector3D interactionMove;
  interactionMove[0] = m_CurrentPickedPoint[0] - m_InitialPickedPoint[0];
  interactionMove[1] = m_CurrentPickedPoint[1] - m_InitialPickedPoint[1];
  interactionMove[2] = m_CurrentPickedPoint[2] - m_InitialPickedPoint[2];

  Point3D origin = m_OriginalGeometry->GetOrigin();

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  if ((interactionEvent->GetSender()) != NULL)
    timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());

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
      vtkDataArray* normal = polyData->GetPointData()->GetVectors( "planeNormal" );
      if (normal != NULL)
      {
        m_ObjectNormal[0] = normal->GetComponent( 0, 0 );
        m_ObjectNormal[1] = normal->GetComponent( 0, 1 );
        m_ObjectNormal[2] = normal->GetComponent( 0, 2 );
      }
    }
  }

  Vector3D transformedObjectNormal;
  this->GetDataNode()->GetData()->GetGeometry( timeStep )->IndexToWorld(
    m_ObjectNormal, transformedObjectNormal );

  this->GetDataNode()->GetData()->GetGeometry( timeStep )->SetOrigin(
    origin + transformedObjectNormal * (interactionMove * transformedObjectNormal) );

  return true;
}
bool mitk::AffineDataInteractor3D::RotateObject (StateMachineAction*, InteractionEvent*)
{
  return true;
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

  vtkPointData *pointData = polyData->GetPointData();
  if (pointData == NULL)
  {
    MITK_ERROR << "AffineInteractor3D: No point data present!";
    return false;
  }

  vtkDataArray *scalars = pointData->GetScalars();
  if (scalars == NULL)
  {
    MITK_ERROR << "AffineInteractor3D: No scalars for point data present!";
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
