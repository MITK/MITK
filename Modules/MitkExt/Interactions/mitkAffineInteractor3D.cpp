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


#include "mitkAffineInteractor3D.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
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
#include "mitkRotationOperation.h"

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>


namespace mitk
{
  
//how precise must the user pick the point
//default value
AffineInteractor3D
::AffineInteractor3D(const char * type, DataTreeNode* dataTreeNode, int /* n */ )
: Interactor( type, dataTreeNode ),
  m_Precision( 6.5 ),
  m_InteractionMode( INTERACTION_MODE_TRANSLATION )
{
  m_OriginalGeometry = Geometry3D::New();

  // Initialize vector arithmetic
  m_ObjectNormal[0] = 0.0;
  m_ObjectNormal[1] = 0.0;
  m_ObjectNormal[2] = 1.0;
}


AffineInteractor3D::~AffineInteractor3D()
{
}


void AffineInteractor3D::SetInteractionMode( unsigned int interactionMode )
{
  m_InteractionMode = interactionMode;
}


void AffineInteractor3D::SetInteractionModeToTranslation()
{
  m_InteractionMode = INTERACTION_MODE_TRANSLATION;
}


void AffineInteractor3D::SetInteractionModeToRotation()
{
  m_InteractionMode = INTERACTION_MODE_ROTATION;
}


unsigned int AffineInteractor3D::GetInteractionMode() const
{
  return m_InteractionMode;
}


void AffineInteractor3D::SetPrecision( ScalarType precision )
{
  m_Precision = precision;
}

// Overwritten since this class can handle it better!
float AffineInteractor3D
::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnValue = 0.5;
  

  // If it is a key event that can be handled in the current state,
  // then return 0.5
  DisplayPositionEvent const *disPosEvent =
    dynamic_cast <const DisplayPositionEvent *> (stateEvent->GetEvent());

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
  //if (stateEvent->GetEvent()->GetType() == Type_MouseMove)
  //{
  //  return 0.0;
  //}

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnValue = 0.5;//it can be understood
  }

  int timeStep = disPosEvent->GetSender()->GetTimeStep();

  //CurveModel *curveModel = dynamic_cast<CurveModel *>(
  //  m_DataTreeNode->GetData() );

  //if ( curveModel != NULL )
  //{
  //  // Get the Geometry2D of the window the user interacts with (for 2D point 
  //  // projection)
  //  BaseRenderer *renderer = stateEvent->GetEvent()->GetSender();
  //  const Geometry2D *projectionPlane = renderer->GetCurrentWorldGeometry2D();

  //  // For reading on the points, Ids etc
  //  //CurveModel::PointSetType *pointSet = curveModel->GetPointSet( timeStep );
  //  //if ( pointSet == NULL )
  //  //{
  //  //  return 0.0;
  //  //}

  //}
  return returnValue;
}


bool AffineInteractor3D
::ExecuteAction( Action *action, StateEvent const *stateEvent )
{
  bool ok = false;

  // Get data object
  BaseData *data = m_DataTreeNode->GetData();
  if ( data == NULL )
  {
    MITK_ERROR << "No data object present!";
    return ok;
  }

  // Get Event and extract renderer
  const Event *event = stateEvent->GetEvent();
  BaseRenderer *renderer = NULL;
  vtkRenderWindow *renderWindow = NULL;
  vtkRenderWindowInteractor *renderWindowInteractor = NULL;
  vtkRenderer *currentVtkRenderer = NULL;
  vtkCamera *camera = NULL;

  if ( event != NULL )
  {
    renderer = event->GetSender();
    if ( renderer != NULL )
    {
      renderWindow = renderer->GetRenderWindow();
      if ( renderWindow != NULL )
      {
        renderWindowInteractor = renderWindow->GetInteractor();
        if ( renderWindowInteractor != NULL )
        {
          currentVtkRenderer = renderWindowInteractor
            ->GetInteractorStyle()->GetCurrentRenderer();
          if ( currentVtkRenderer != NULL )
          {
            camera = currentVtkRenderer->GetActiveCamera();
          }
        }
      }
    }
  }

  // Check if we have a DisplayPositionEvent
  const DisplayPositionEvent *dpe = 
    dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );     
  if ( dpe != NULL )
  {
    m_CurrentPickedPoint = dpe->GetWorldPosition();
    m_CurrentPickedDisplayPoint = dpe->GetDisplayPosition();
  }

  // Get the timestep to also support 3D+t
  int timeStep = 0;
  ScalarType timeInMS = 0.0;
  if ( renderer != NULL )
  {
    timeStep = renderer->GetTimeStep( data );
    timeInMS = renderer->GetTime();
  }

  // If data is an mitk::Surface, extract it
  Surface *surface = dynamic_cast< Surface * >( data );
  vtkPolyData *polyData = NULL;
  if ( surface != NULL )
  {
    polyData = surface->GetVtkPolyData( timeStep );

    // Extract surface normal from surface (if existent, otherwise use default)
    vtkPointData *pointData = polyData->GetPointData();
    if ( pointData != NULL )
    {
      vtkDataArray *normal = polyData->GetPointData()->GetVectors( "planeNormal" );
      if ( normal != NULL )
      {
        m_ObjectNormal[0] = normal->GetComponent( 0, 0 );
        m_ObjectNormal[1] = normal->GetComponent( 0, 1 );
        m_ObjectNormal[2] = normal->GetComponent( 0, 2 );
      }
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
      // Re-enable VTK interactor (may have been disabled previously)
      if ( renderWindowInteractor != NULL )
      {
        renderWindowInteractor->Enable();
      }

      // Check if we have a DisplayPositionEvent
      const DisplayPositionEvent *dpe = 
        dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );     
      if ( dpe == NULL )
      {
        ok = true;
        break;
      }

      // Check if an object is present at the current mouse position
      DataTreeNode *pickedNode = dpe->GetPickedObjectNode();
      StateEvent *newStateEvent;
      if ( pickedNode == m_DataTreeNode )
      {
        // Yes: object will be selected
        newStateEvent = new StateEvent( EIDYES );
      }
      else
      {
        // No: back to start state
        newStateEvent = new StateEvent( EIDNO );
      }

      this->HandleEvent( newStateEvent );

      ok = true;
      break;
    }

  case AcDESELECTOBJECT:
    {
      // Color object white
      m_DataTreeNode->SetColor( 1.0, 1.0, 1.0 );
      RenderingManager::GetInstance()->RequestUpdateAll();
  
      // Colorize surface / wireframe as inactive
      this->ColorizeSurface( polyData,
        m_CurrentPickedPoint, -1.0 );

      ok = true;
      break;
    }

  case AcSELECTPICKEDOBJECT:
    {
      // Color object red
      m_DataTreeNode->SetColor( 1.0, 0.0, 0.0 );
      RenderingManager::GetInstance()->RequestUpdateAll();

      // Colorize surface / wireframe dependend on distance from picked point
      this->ColorizeSurface( polyData,
        m_CurrentPickedPoint, 0.0 );

      ok = true;  
      break;
    }

  case AcINITMOVE:
    {
      // Disable VTK interactor until MITK interaction has been completed
      if ( renderWindowInteractor != NULL )
      {
        renderWindowInteractor->Disable();
      }

      // Check if we have a DisplayPositionEvent
      const DisplayPositionEvent *dpe = 
        dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );     
      if ( dpe == NULL )
      {
        ok = true;
        break;
      }

      DataTreeNode *pickedNode = dpe->GetPickedObjectNode();
      
      m_InitialPickedPoint = m_CurrentPickedPoint;
      m_InitialPickedDisplayPoint = m_CurrentPickedDisplayPoint;

      if ( currentVtkRenderer != NULL )
      {
        vtkInteractorObserver::ComputeDisplayToWorld(
          currentVtkRenderer,
          m_InitialPickedDisplayPoint[0],
          m_InitialPickedDisplayPoint[1],
          0.0, //m_InitialInteractionPickedPoint[2],
          m_InitialPickedPointWorld );
      }


      // Make deep copy of current Geometry3D of the plane
      data->UpdateOutputInformation(); // make sure that the Geometry is up-to-date
      m_OriginalGeometry = static_cast< Geometry3D * >( 
        data->GetGeometry( timeStep )->Clone().GetPointer() );

      ok = true;
      break;
    }

  case AcMOVE:
    {
      // Check if we have a DisplayPositionEvent
      const DisplayPositionEvent *dpe = 
        dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );     
      if ( dpe == NULL )
      {
        ok = true;
        break;
      }

      if ( currentVtkRenderer != NULL )
      {
        vtkInteractorObserver::ComputeDisplayToWorld(
          currentVtkRenderer,
          m_CurrentPickedDisplayPoint[0],
          m_CurrentPickedDisplayPoint[1],
          0.0, //m_InitialInteractionPickedPoint[2],
          m_CurrentPickedPointWorld );
      }


      Vector3D interactionMove;
      interactionMove[0] = m_CurrentPickedPointWorld[0] - m_InitialPickedPointWorld[0];
      interactionMove[1] = m_CurrentPickedPointWorld[1] - m_InitialPickedPointWorld[1];
      interactionMove[2] = m_CurrentPickedPointWorld[2] - m_InitialPickedPointWorld[2];

      if ( m_InteractionMode == INTERACTION_MODE_TRANSLATION )
      {
        Point3D origin = m_OriginalGeometry->GetOrigin();

        Vector3D transformedObjectNormal;
        data->GetGeometry( timeStep )->IndexToWorld(
          origin, m_ObjectNormal, transformedObjectNormal );

        data->GetGeometry( timeStep )->SetOrigin(
          origin + transformedObjectNormal * (interactionMove * transformedObjectNormal) );
      }
      else if ( m_InteractionMode == INTERACTION_MODE_ROTATION )
      {
        if ( camera )
        {
          vtkFloatingPointType vpn[3];
          camera->GetViewPlaneNormal( vpn );

          Vector3D viewPlaneNormal;
          viewPlaneNormal[0] = vpn[0];
          viewPlaneNormal[1] = vpn[1];
          viewPlaneNormal[2] = vpn[2];

          Vector3D rotationAxis = 
            itk::CrossProduct( viewPlaneNormal, interactionMove );
          rotationAxis.Normalize();

          int *size = currentVtkRenderer->GetSize();
          double l2 =
            (m_CurrentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) *
            (m_CurrentPickedDisplayPoint[0] - m_InitialPickedDisplayPoint[0]) +
            (m_CurrentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]) *
            (m_CurrentPickedDisplayPoint[1] - m_InitialPickedDisplayPoint[1]);

          double rotationAngle = 360.0 * sqrt(l2/(size[0]*size[0]+size[1]*size[1]));

          // Use center of data bounding box as center of rotation
          Point3D rotationCenter = m_OriginalGeometry->GetCenter();;

          // Reset current Geometry3D to original state (pre-interaction) and
          // apply rotation
          RotationOperation op( OpROTATE, rotationCenter, rotationAxis, rotationAngle );
          Geometry3D::Pointer newGeometry = static_cast< Geometry3D * >( 
            m_OriginalGeometry->Clone().GetPointer() );
          newGeometry->ExecuteOperation( &op );
          data->SetGeometry( newGeometry, timeStep );

        }
      }

      RenderingManager::GetInstance()->RequestUpdateAll();
      ok = true;
      break;
    }



  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}

bool AffineInteractor3D::ColorizeSurface( vtkPolyData *polyData, 
  const Point3D & /*pickedPoint*/, double scalar )
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

  for ( unsigned int i = 0; i < pointData->GetNumberOfTuples(); ++i )
  {
    scalars->SetComponent( i, 0, scalar );
  }

  polyData->Modified();
  pointData->Update();

  return true;
}


} // namespace