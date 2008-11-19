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


#include "mitkPointNavigationController.h"
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
#include <mitkVtkLayerController.h>

namespace mitk {

PointNavigationController::PointNavigationController( const char *type ) 
: NavigationController( type )
{
}


PointNavigationController::~PointNavigationController()
{
}


void PointNavigationController::Update()
{
  if ( !m_BlockUpdate )
  {
    if ( m_ViewDirection == Transversal )
    {
      this->Update( Transversal, false, false, true );
    }
    else
    {
      this->Update( m_ViewDirection );
    }
  }
} 

void
PointNavigationController::Update(
  PointNavigationController::ViewDirection viewDirection,
  bool top, bool frontside, bool rotated )
{
  const TimeSlicedGeometry* worldTimeSlicedGeometry =
    dynamic_cast< const TimeSlicedGeometry * >(
      m_InputWorldGeometry.GetPointer() );

  if( m_BlockUpdate || 
      m_InputWorldGeometry.IsNull() || 
      ( (worldTimeSlicedGeometry != NULL) && (worldTimeSlicedGeometry->GetTimeSteps() == 0) )
    )
  {
    return;
  }

  m_BlockUpdate = true;

  if ( m_LastUpdateTime < m_InputWorldGeometry->GetMTime() )
  {
    Modified();
  }

  this->SetViewDirection( viewDirection );
  this->SetTop( top );
  this->SetFrontSide( frontside );
  this->SetRotated( rotated );

  if ( m_LastUpdateTime < GetMTime() )
  {
    m_LastUpdateTime = GetMTime();

    // initialize the viewplane
    SlicedGeometry3D::Pointer slicedWorldGeometry = NULL;
    
    m_CreatedWorldGeometry = NULL;
    switch ( viewDirection )
    {
    case Original:
      if ( worldTimeSlicedGeometry != NULL )
      {
        m_CreatedWorldGeometry = static_cast< TimeSlicedGeometry * >(
          m_InputWorldGeometry->Clone().GetPointer() );

        worldTimeSlicedGeometry = m_CreatedWorldGeometry.GetPointer();

        slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
          m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos() ) );

        if ( slicedWorldGeometry.IsNotNull() )
        {
          break;
        }
      }
      else
      {
        const SlicedGeometry3D *worldSlicedGeometry =
          dynamic_cast< const SlicedGeometry3D * >(
            m_InputWorldGeometry.GetPointer());

        if ( worldSlicedGeometry != NULL )
        {
          slicedWorldGeometry = static_cast< SlicedGeometry3D * >(
            m_InputWorldGeometry->Clone().GetPointer());
          break;
        }
      }
      //else: use Transversal: no "break" here!!

    case Transversal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(
        m_InputWorldGeometry, PlaneGeometry::Transversal,
        top, frontside, rotated );
      slicedWorldGeometry->SetNavigationController( this );
      break;

    case Frontal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes( m_InputWorldGeometry,
        PlaneGeometry::Frontal, top, frontside, rotated );
      slicedWorldGeometry->SetNavigationController( this );
      break;

    case Sagittal:
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes( m_InputWorldGeometry,
        PlaneGeometry::Sagittal, top, frontside, rotated );
      slicedWorldGeometry->SetNavigationController( this );
      break;
    default:
      itkExceptionMacro("unknown ViewDirection");
    }

    m_Slice->SetPos( 0 );
    m_Slice->SetSteps( (int)slicedWorldGeometry->GetSlices() );
  
    if ( m_CreatedWorldGeometry.IsNull() )
    {
      // initialize TimeSlicedGeometry
      m_CreatedWorldGeometry = TimeSlicedGeometry::New();
    }
    if ( worldTimeSlicedGeometry == NULL )
    {
      m_CreatedWorldGeometry->InitializeEvenlyTimed( slicedWorldGeometry, 1 );
      m_Time->SetSteps( 0 );
      m_Time->SetPos( 0 );
      m_Time->InvalidateRange();
    }
    else
    {
      m_BlockUpdate = true;
      m_Time->SetSteps( worldTimeSlicedGeometry->GetTimeSteps() );
      m_Time->SetPos( 0 );

      const TimeBounds &timeBounds = worldTimeSlicedGeometry->GetTimeBounds();
      m_Time->SetRange( timeBounds[0], timeBounds[1] );

      m_BlockUpdate = false;

      assert( worldTimeSlicedGeometry->GetGeometry3D( this->GetTime()->GetPos() ) != NULL );

      slicedWorldGeometry->SetTimeBounds(
        worldTimeSlicedGeometry->GetGeometry3D( this->GetTime()->GetPos() )->GetTimeBounds() );

      //@todo implement for non-evenly-timed geometry!
      m_CreatedWorldGeometry->InitializeEvenlyTimed(
        slicedWorldGeometry, worldTimeSlicedGeometry->GetTimeSteps() );
    }
  }

  // unblock update; we may do this now, because if m_BlockUpdate was already
  // true before this method was entered, then we will never come here.
  m_BlockUpdate = false;

  // Send the geometry. Do this even if nothing was changed, because maybe
  // Update() was only called to re-send the old geometry and time/slice data.
  this->SendCreatedWorldGeometry();
  this->SendSlice();
  this->SendTime();

  // Adjust the stepper range of slice stepper according to geometry
  this->AdjustSliceStepperRange();
}

void
PointNavigationController::SendCreatedWorldGeometry()
{
  // Send the geometry. Do this even if nothing was changed, because maybe
  // Update() was only called to re-send the old geometry.
  if ( !m_BlockUpdate )
  {
    this->InvokeEvent( GeometrySendEvent(m_CreatedWorldGeometry, 0) );
  }
}

void
PointNavigationController::SendCreatedWorldGeometryUpdate()
{
  if ( !m_BlockUpdate )
  {
    this->InvokeEvent(
      GeometryUpdateEvent(m_CreatedWorldGeometry, m_Slice->GetPos()) );
  }
}

void
PointNavigationController::SendSlice()
{
  if ( !m_BlockUpdate )
  {
    if ( m_CreatedWorldGeometry.IsNotNull() )
    {
      this->InvokeEvent(
        GeometrySliceEvent(m_CreatedWorldGeometry, m_Slice->GetPos()) );

      // Request rendering update for all views
      this->GetRenderingManager()->RequestUpdateAll();
    }
  }
}

void
PointNavigationController::SendTime()
{
  if ( !m_BlockUpdate )
  {
    if ( m_CreatedWorldGeometry.IsNotNull() )
    {
      this->InvokeEvent(
        GeometryTimeEvent(m_CreatedWorldGeometry, m_Time->GetPos()) );

      // Request rendering update for all views
      this->GetRenderingManager()->RequestUpdateAll();
    }
  }
}


void
PointNavigationController
::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(
      &geometrySliceEvent);
  assert(sliceEvent!=NULL);

  this->GetSlice()->SetPos(sliceEvent->GetPos());
}

//void PointNavigationController::setSphere()
//{
//  //@todo add time to PositionEvent and use here!!
//  //SlicedGeometry3D* slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
//  //  m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos()) );
//
//  //mitk:DataTreeNode::Pointer node = mitk::DataStorage::GetInstance()->GetNamedNode("Localizer");
//  //if (node)
//  //  //node->GetData()->GetGeometry()->SetOrigin();
//  //this->SendCreatedWorldGeometryUpdate();
//}

void PointNavigationController::setSphere(const Point3D &point)
{
  //@todo add time to PositionEvent and use here!!
  //SlicedGeometry3D* slicedWorldGeometry = dynamic_cast< SlicedGeometry3D * >(
  //  m_CreatedWorldGeometry->GetGeometry3D( this->GetTime()->GetPos()) );

  
  DataTreeNode::Pointer node = mitk::DataStorage::GetInstance()->GetNamedNode("Localizer");
  if (node)
    node->GetData()->GetGeometry()->SetOrigin(point);
  this->SendCreatedWorldGeometryUpdate();

}

void
PointNavigationController::ReorientSlices( const Point3D &point, 
  const Vector3D &normal )
{
  PlaneOperation op( OpORIENT, point, normal );

  m_CreatedWorldGeometry->ExecuteOperation( &op );

  this->SendCreatedWorldGeometryUpdate();
}

void
PointNavigationController::ExecuteOperation( Operation *operation )
{
  // switch on type
  // - select best slice for a given point
  // - rotate created world geometry according to Operation->SomeInfo()
  if ( !operation )
  {
    return;
  }
  
  switch ( operation->GetOperationType() )
  {
    case OpMOVE: // should be a point operation
    {
      if ( !m_SliceLocked ) //do not move the cross position
      {
        // set the point
        PointOperation *po = dynamic_cast< PointOperation * >( operation );
        if ( po && po->GetIndex() == -1 )
        {
          this-> setSphere( po->GetPoint() );
        }
        else if ( po && po->GetIndex() != -1 ) // undo case because index != -1, index holds the old position of this slice
        {
          this->GetSlice()->SetPos( po->GetIndex() );
        }
      }
      break;
    }
    default:
    {
      // do nothing
      break;
    }
  }
}

// Relict from the old times, when automous descisions were accepted
// behaviour. Remains in here, because some RenderWindows do exist outide
// of StdMultiWidgets.
bool
PointNavigationController
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
              OperationEvent *operationEvent = new mitk::OperationEvent(this, doOp, undoOp, "Move Point");
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

