/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-02 16:21:08 +0200 (Do, 02 Okt 2008) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSurfaceDeformationInteractor3D.h"
#include "mitkPointOperation.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkWheelEvent.h"
#include "mitkStatusBar.h"
#include "mitkDataTreeNode.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkStateEvent.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkStateMachineFactory.h"
#include "mitkStateTransitionOperation.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkSurface.h"


#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyle.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>


//how precise must the user pick the point
//default value
mitk::SurfaceDeformationInteractor3D
::SurfaceDeformationInteractor3D(const char * type, DataTreeNode* dataTreeNode, int /* n */ )
: Interactor( type, dataTreeNode ),
  m_Precision( 6.5 ),
  m_PickedSurfaceNode( NULL ),
  m_PickedSurface( NULL ),
  m_GaussSigma( 30.0 )
{
  m_OriginalPolyData = vtkPolyData::New();

  // Initialize vector arithmetic
  m_ObjectNormal[0] = 0.0;
  m_ObjectNormal[1] = 0.0;
  m_ObjectNormal[2] = 1.0;
}

mitk::SurfaceDeformationInteractor3D::~SurfaceDeformationInteractor3D()
{
  m_OriginalPolyData->Delete();
}

void mitk::SurfaceDeformationInteractor3D::SetPrecision( mitk::ScalarType precision )
{
  m_Precision = precision;
}

// Overwritten since this class can handle it better!
float mitk::SurfaceDeformationInteractor3D
::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnValue = 0.5;
  

  // If it is a key event that can be handled in the current state,
  // then return 0.5
  mitk::DisplayPositionEvent const *disPosEvent =
    dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  // Key event handling:
  if (disPosEvent == NULL)
  {
    // Check if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0.0;
    }
  }

  //on MouseMove do nothing!
  //if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  //{
  //  return 0.0;
  //}

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnValue = 0.5;//it can be understood
  }

  int timeStep = disPosEvent->GetSender()->GetTimeStep();

  //mitk::CurveModel *curveModel = dynamic_cast<mitk::CurveModel *>(
  //  m_DataTreeNode->GetData() );

  //if ( curveModel != NULL )
  //{
  //  // Get the Geometry2D of the window the user interacts with (for 2D point 
  //  // projection)
  //  mitk::BaseRenderer *renderer = stateEvent->GetEvent()->GetSender();
  //  const Geometry2D *projectionPlane = renderer->GetCurrentWorldGeometry2D();

  //  // For reading on the points, Ids etc
  //  //mitk::CurveModel::PointSetType *pointSet = curveModel->GetPointSet( timeStep );
  //  //if ( pointSet == NULL )
  //  //{
  //  //  return 0.0;
  //  //}

  //}
  return returnValue;
}


bool mitk::SurfaceDeformationInteractor3D
::ExecuteAction( Action *action, mitk::StateEvent const *stateEvent )
{
  bool ok = false;

  // Get data object
  mitk::BaseData *data = m_DataTreeNode->GetData();
  if ( data == NULL )
  {
    MITK_ERROR << "No data object present!";
    return ok;
  }

  // Get mitk::Event and extract renderer
  const mitk::Event *event = stateEvent->GetEvent();
  mitk::BaseRenderer *renderer = NULL;
  vtkRenderWindow *renderWindow = NULL;
  vtkRenderWindowInteractor *renderWindowInteractor = NULL;

  if ( event != NULL )
  {
    renderer = event->GetSender();
    if ( renderer != NULL )
    {
      renderWindow = renderer->GetRenderWindow();
      if ( renderWindow != NULL )
      {
        renderWindowInteractor = renderWindow->GetInteractor();
      }
    }
  }

  // Check if we have a DisplayPositionEvent
  const mitk::DisplayPositionEvent *dpe = 
    dynamic_cast< const mitk::DisplayPositionEvent * >( stateEvent->GetEvent() );     
  if ( dpe != NULL )
  {
    m_PickedSurfaceNode = dpe->GetPickedObjectNode();
    m_CurrentPickedPoint = dpe->GetWorldPosition();
    m_CurrentPickedDisplayPoint = dpe->GetDisplayPosition();
  }


  // Get the timestep to also support 3D+t
  int timeStep = 0;
  mitk::ScalarType timeInMS = 0.0;
  if ( renderer != NULL )
  {
    timeStep = renderer->GetTimeStep( data );
    timeInMS = renderer->GetTime();
  }

  // Extract surface
  m_Surface = dynamic_cast< Surface * >( data );
  if ( m_Surface != NULL )
  {
    m_PolyData = m_Surface->GetVtkPolyData( timeStep );
  }
  else
  {
    m_PolyData = NULL;
  }

  // Extract surface normal from surface (if existent, otherwise use default)
  vtkPointData *pointData = m_PolyData->GetPointData();
  if ( pointData != NULL )
  {
    vtkDataArray *normal = m_PolyData->GetPointData()->GetVectors( "planeNormal" );
    if ( normal != NULL )
    {
      m_ObjectNormal[0] = normal->GetComponent( 0, 0 );
      m_ObjectNormal[1] = normal->GetComponent( 0, 1 );
      m_ObjectNormal[2] = normal->GetComponent( 0, 2 );
    }
  }

  // Get geometry object
  m_Geometry = data->GetGeometry( timeStep );


  // Make sure that the data (if time-resolved) has enough entries;
  // if not, create the required extra ones (empty)
  data->Expand( timeStep+1 );


  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;


  case AcCHECKOBJECT:
    {
      // Check if an object is present at the current mouse position
      m_PickedSurface = NULL;
      m_PickedPolyData = NULL;

      if ( m_PickedSurfaceNode != NULL )
      {
        m_PickedSurface = dynamic_cast< mitk::Surface * >( m_PickedSurfaceNode->GetData() );
        if ( m_PickedSurface != NULL )
        {
          m_PickedPolyData = m_PickedSurface->GetVtkPolyData( timeStep );
        }
      }

      mitk::StateEvent *newStateEvent;
      if ( (m_PickedSurfaceNode == m_DataTreeNode) && (m_PickedSurface != NULL) )
      {
        // Yes: object will be selected
        newStateEvent = new mitk::StateEvent( EIDYES );
      
        // Disable VTK interactor until MITK interaction has been completed
        if ( renderWindowInteractor != NULL )
        {
          renderWindowInteractor->Disable();
        }
      }
      else
      {
        // No: back to start state
        newStateEvent = new mitk::StateEvent( EIDNO );

        // Re-enable VTK interactor (may have been disabled previously)
        if ( renderWindowInteractor != NULL )
        {
          renderWindowInteractor->Enable();
        }
      }

      this->HandleEvent( newStateEvent );

      // Colorized surface at current picked position
      m_SurfaceColorizationCenter = m_CurrentPickedPoint;

      ok = true;
      break;
    }

  case AcDESELECTOBJECT:
    {
      // Color object white
      m_DataTreeNode->SetColor( 1.0, 1.0, 1.0 );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      // Colorize surface / wireframe as inactive
      this->ColorizeSurface( m_PolyData,
        m_SurfaceColorizationCenter, COLORIZATION_CONSTANT, -1.0 );

      ok = true;
      break;
    }

  case AcSELECTPICKEDOBJECT:
    {
      // Color object red
      m_DataTreeNode->SetColor( 1.0, 0.0, 0.0 );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      // Colorize surface / wireframe dependend on distance from picked point
      this->ColorizeSurface( m_PolyData,
        m_SurfaceColorizationCenter, COLORIZATION_GAUSS );

      ok = true;  
      break;
    }

  case AcINITMOVE:
    {
      // Store current picked point
      m_InitialPickedPoint = m_CurrentPickedPoint;
      m_InitialPickedDisplayPoint = m_CurrentPickedDisplayPoint;

      if ( renderWindowInteractor != NULL )
      {
        vtkInteractorObserver::ComputeDisplayToWorld(
          renderWindowInteractor->GetInteractorStyle()->GetCurrentRenderer(),
          m_InitialPickedDisplayPoint[0],
          m_InitialPickedDisplayPoint[1],
          0.0, //m_InitialInteractionPickedPoint[2],
          m_InitialPickedPointWorld );
      }


      // Make deep copy of vtkPolyData interacted on
      m_OriginalPolyData->DeepCopy( m_PolyData );

      ok = true;
      break;
    }

  case AcMOVE:
    {
      if ( renderWindowInteractor != NULL )
      {
        vtkInteractorObserver::ComputeDisplayToWorld(
          renderWindowInteractor->GetInteractorStyle()->GetCurrentRenderer(),
          m_CurrentPickedDisplayPoint[0],
          m_CurrentPickedDisplayPoint[1],
          0.0, //m_InitialInteractionPickedPoint[2],
          m_CurrentPickedPointWorld );
      }


      // Calculate mouse move in 3D space
      mitk::Vector3D interactionMove;
      interactionMove[0] = m_CurrentPickedPointWorld[0] - m_InitialPickedPointWorld[0];
      interactionMove[1] = m_CurrentPickedPointWorld[1] - m_InitialPickedPointWorld[1];
      interactionMove[2] = m_CurrentPickedPointWorld[2] - m_InitialPickedPointWorld[2];

      // Transform mouse move into geometry space
      data->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
      mitk::Point3D origin; origin.Fill( 0.0 );
      m_Geometry->WorldToIndex( origin, interactionMove, interactionMove );


      // Get picked point and transform into local coordinates
      mitk::Point3D pickedPoint;
      m_Geometry->WorldToIndex( m_InitialPickedPoint, pickedPoint );
      mitk::Vector3D v1 = pickedPoint.GetVectorFromOrigin();

      mitk::Vector3D v2 = m_ObjectNormal * (interactionMove * m_ObjectNormal);


      vtkPoints *originalPoints = m_OriginalPolyData->GetPoints();
      vtkPoints *deformedPoints = m_PolyData->GetPoints();

      double denom = m_GaussSigma * m_GaussSigma * 2;
      double point[3];
      for ( unsigned int i = 0; i < deformedPoints->GetNumberOfPoints(); ++i )
      {
        // Get original point
        vtkFloatingPointType *originalPoint = originalPoints->GetPoint( i );
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

      // Make sure that surface is colorized at initial picked position
      // as long as we are in deformation state
      m_SurfaceColorizationCenter = m_InitialPickedPoint;

      m_PolyData->Modified();
      m_Surface->Modified();

      mitk::RenderingManager::GetInstance()->RequestUpdateAll(
        mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
      ok = false;
      break;
    }

  case AcMODIFY:
    {
      // Check if we have an mitk::WheelEvent
      const mitk::WheelEvent *we = 
        dynamic_cast< const mitk::WheelEvent * >( stateEvent->GetEvent() );     
      if ( we == NULL )
      {
        ok = true;
        break;
      }

      m_GaussSigma += (double) (we->GetDelta()) / 20;;
      if ( m_GaussSigma < 10.0 )
      {
        m_GaussSigma = 10.0;
      }
      else if ( m_GaussSigma > 128.0 )
      {
        m_GaussSigma = 128.0;
      }

      // Colorize surface / wireframe dependend on sigma and distance from picked point
      this->ColorizeSurface( m_PolyData,
        m_SurfaceColorizationCenter, COLORIZATION_GAUSS );

      
      mitk::RenderingManager::GetInstance()->RequestUpdateAll(
        mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS );
      ok = true;
      break;
    }


  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}


bool mitk::SurfaceDeformationInteractor3D::ColorizeSurface( vtkPolyData *polyData, 
  const Point3D &pickedPoint, int mode, double scalar )
{
  if ( polyData == NULL )
  {
    return false;
  }

  vtkPoints *points = polyData->GetPoints();
  vtkPointData *pointData = polyData->GetPointData();
  if ( pointData == NULL )
  {
    return false;
  }

  vtkDataArray *scalars = pointData->GetScalars();
  if ( scalars == NULL )
  {
    return false;
  }

  if ( mode == COLORIZATION_GAUSS )
  {
    // Get picked point and transform into local coordinates
    mitk::Point3D localPickedPoint;
    m_Geometry->WorldToIndex( pickedPoint, localPickedPoint );
    mitk::Vector3D v1 = localPickedPoint.GetVectorFromOrigin();

    double denom = m_GaussSigma * m_GaussSigma * 2;
    for ( unsigned int i = 0; i < points->GetNumberOfPoints(); ++i )
    {
      // Get original point
      vtkFloatingPointType *point = points->GetPoint( i );
      mitk::Vector3D v0;
      v0[0] = point[0];
      v0[1] = point[1];
      v0[2] = point[2];

      // Calculate distance of this point from line through picked point
      double d = itk::CrossProduct( m_ObjectNormal, (v1 - v0) ).GetNorm();
      double t = exp( - d * d / denom );

      scalars->SetComponent( i, 0, t );
    }
  }
  else if ( mode == COLORIZATION_CONSTANT )
  {
    for ( unsigned int i = 0; i < pointData->GetNumberOfTuples(); ++i )
    {
      scalars->SetComponent( i, 0, scalar );
    }
  }

  polyData->Modified();
  pointData->Update();

  return true;
}
