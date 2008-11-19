/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-21 11:55:15 +0200 (Mo, 21 Apr 2008) $
Version:   $Revision: 14134 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkGlobalInteraction.h"
#include "mitkEventMapper.h"
#include "mitkFocusManager.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkPlaneOperation.h"
#include "mitkUndoController.h"
#include "mitkOperationEvent.h"

#include <itkCommand.h>

namespace mitk {

NavigationController::NavigationController( const char *type ) 
: BaseController( type ),
  m_InputWorldGeometry( NULL ),
  m_CreatedWorldGeometry( NULL ), 
  m_ViewDirection( Transversal ),
  m_DefaultViewDirection( Transversal ),
  m_Renderer( NULL ),
  m_Top( false ),
  m_FrontSide( false ),
  m_Rotated( false ),
  m_BlockUpdate( false ),
  m_SliceLocked( false ),
  m_SliceRotationLocked( false ),
  m_OldPos(0)
{
  m_Top = false;
  m_FrontSide = false;
  m_Rotated = false;
}


NavigationController::~NavigationController()
{
}


void
NavigationController::SetInputWorldGeometry( const Geometry3D *geometry )
{
  if ( geometry != NULL )
  {
    if ( const_cast< BoundingBox * >( geometry->GetBoundingBox())
         ->GetDiagonalLength2() < eps )
    {
      itkWarningMacro( "setting an empty bounding-box" );
      geometry = NULL;
    }
  }
  if ( m_InputWorldGeometry != geometry )
  {
    m_InputWorldGeometry = geometry;
    this->Modified();
  }
}

RenderingManager *
NavigationController::GetRenderingManager() const
{
  mitk::RenderingManager* renderingManager = m_RenderingManager.GetPointer();
  if(renderingManager == NULL)
    return mitk::RenderingManager::GetInstance();
  return renderingManager;
}


void NavigationController::SetViewDirectionToDefault()
{
  m_ViewDirection = m_DefaultViewDirection;
}

void
NavigationController::SetGeometry( const itk::EventObject & )
{
}


const mitk::TimeSlicedGeometry *
NavigationController::GetCreatedWorldGeometry()
{
  return m_CreatedWorldGeometry;
}


const mitk::Geometry3D *
NavigationController::GetCurrentGeometry3D()
{
  if ( m_CreatedWorldGeometry.IsNotNull() )
  {
    return m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos() );
  }
  else
  {
    return NULL;
  }
}


const mitk::PlaneGeometry *
NavigationController::GetCurrentPlaneGeometry()
{
  const mitk::SlicedGeometry3D *slicedGeometry = 
    dynamic_cast< const mitk::SlicedGeometry3D * >
      ( this->GetCurrentGeometry3D() );

  if ( slicedGeometry )
  {
    const mitk::PlaneGeometry *planeGeometry = 
      dynamic_cast< mitk::PlaneGeometry * >
        ( slicedGeometry->GetGeometry2D(this->GetSlice()->GetPos()) );
    return planeGeometry;
  }
  else
  {
    return NULL;
  }
}


void 
NavigationController::SetRenderer( BaseRenderer *renderer )
{
  m_Renderer = renderer;
}

BaseRenderer *
NavigationController::GetRenderer() const
{
  return m_Renderer;
}



void
NavigationController::AdjustSliceStepperRange()
{
  const mitk::SlicedGeometry3D *slicedGeometry = 
    dynamic_cast< const mitk::SlicedGeometry3D * >
      ( this->GetCurrentGeometry3D() );

  const Vector3D &direction = slicedGeometry->GetDirectionVector();

  int c = 0;
  int i, k = 0;
  for ( i = 0; i < 3; ++i )
  {
    if ( fabs( (float) direction[i] ) < 0.000000001 ) { ++c; }
    else { k = i; }
  }

  if ( c == 2 )
  {
    ScalarType min = m_InputWorldGeometry->GetOrigin()[k];
    ScalarType max = min + m_InputWorldGeometry->GetExtentInMM( k );

    m_Slice->SetRange( min, max );
  }
  else
  {
    m_Slice->InvalidateRange();
  }

}


// Relict from the old times, when automous descisions were accepted
// behaviour. Remains in here, because some RenderWindows do exist outide
// of StdMultiWidgets.
bool
NavigationController
::ExecuteAction( Action* action, StateEvent const* stateEvent )
{
  bool ok = false;

  const PositionEvent* posEvent = dynamic_cast< const PositionEvent * >(
    stateEvent->GetEvent() );
  if ( posEvent != NULL )
  {
    if ( m_CreatedWorldGeometry.IsNull() )
    {
      return true;
    }
    switch (action->GetActionId())
    {
    case AcMOVE:
      {
        BaseRenderer *baseRenderer = posEvent->GetSender();
        if ( !baseRenderer )
        {
          baseRenderer = const_cast<BaseRenderer *>(
            GlobalInteraction::GetInstance()->GetFocus() );
        }
        if ( baseRenderer )
          if ( baseRenderer->GetMapperID() == 1 )
          {
            PointOperation* doOp = new mitk::PointOperation(OpMOVE, posEvent->GetWorldPosition());
            if (m_UndoEnabled)
            {
              m_OldPos = this->GetSlice()->GetPos();
              // m_OldPos holds the old slice position. For the undo controller this old position will be stored as index in mitk::PointOperation
              PointOperation* undoOp = new mitk::PointOperation(OpMOVE, posEvent->GetWorldPosition(), m_OldPos);
              OperationEvent *operationEvent = new mitk::OperationEvent(this, doOp, undoOp, "Move slices");
              m_UndoController->SetOperationEvent(operationEvent);
            }

            this->ExecuteOperation( doOp ); 
          }
          ok = true;
          break;
      }
    default:
      ok = true;
      break;
    }
    return ok;
  } 

  const DisplayPositionEvent *displPosEvent =
    dynamic_cast< const DisplayPositionEvent * >( stateEvent->GetEvent() );

  if ( displPosEvent != NULL )
  {
    return true;
  }

  return false;
}

} // namespace

