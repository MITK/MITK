/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <mitkSliceNavigationController.h>
#include <mitkBaseRenderer.h>
#include <mitkRenderWindow.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkPlaneGeometry.h>
#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkPositionEvent.h>
#include <mitkInteractionConst.h>
#include <mitkAction.h>
#include <mitkGlobalInteraction.h>
#include <mitkEventMapper.h>
#include <mitkFocusManager.h>
#include <mitkOpenGLRenderer.h>
#include <mitkRenderingManager.h>

#include <mitkInteractionConst.h>
#include <mitkPointOperation.h>

#include <itkCommand.h>

namespace mitk {

SliceNavigationController::SliceNavigationController(const char * type) 
  : BaseController(type), m_InputWorldGeometry(NULL), m_CreatedWorldGeometry(NULL), 
    m_ViewDirection(Transversal), m_BlockUpdate(false), m_SliceLocked(false)
{
  itk::SimpleMemberCommand<SliceNavigationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  timeStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  sliceStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SendSlice);
  timeStepperChangedCommand->SetCallbackFunction(this,  &SliceNavigationController::SendTime);

  m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
  m_Time->AddObserver(itk::ModifiedEvent(),  timeStepperChangedCommand);
}

SliceNavigationController::~SliceNavigationController()
{

}

void SliceNavigationController::SetInputWorldGeometry(const Geometry3D* geometry)
{
  if(geometry != NULL)
  {
    if(const_cast<BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2()<eps)
    {
      itkWarningMacro("setting an empty bounding-box");
      geometry = NULL;
    }
  }
  if(m_InputWorldGeometry != geometry)
  {
    m_InputWorldGeometry = geometry;
    Modified();
  }
}

RenderingManager* SliceNavigationController::GetRenderingManager() const
{
  mitk::RenderingManager* renderingManager = m_RenderingManager.GetPointer();
  if(renderingManager == NULL)
    return mitk::RenderingManager::GetInstance();
  return renderingManager;
}

void SliceNavigationController::Update()
{
  if(m_BlockUpdate)
    return;

  if(m_ViewDirection == Transversal)
  {
    Update(Transversal, false, false, true);
  }
  else
  {
    Update(m_ViewDirection);
  }
} 

void SliceNavigationController::Update(SliceNavigationController::ViewDirection viewDirection, bool top, bool frontside, bool rotated)
{
  if(m_InputWorldGeometry.IsNull())
    return;
  if(m_BlockUpdate)
    return;
  m_BlockUpdate = true;

  if(m_LastUpdateTime < m_InputWorldGeometry->GetMTime())
  {
    Modified();
  }
  SetViewDirection(viewDirection);
  SetTop(top);
  SetFrontSide(frontside);
  SetRotated(rotated);

  if(m_LastUpdateTime < GetMTime())
  {
    m_LastUpdateTime = GetMTime();

    // initialize the viewplane
    PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();

    SlicedGeometry3D::Pointer slicedWorldGeometry = NULL;
    
    m_CreatedWorldGeometry = NULL;
    const TimeSlicedGeometry* worldTimeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(m_InputWorldGeometry.GetPointer());
    switch(viewDirection)
    {
    case Original:
      if(worldTimeSlicedGeometry != NULL)
      {
        m_CreatedWorldGeometry =
          static_cast<TimeSlicedGeometry*>(m_InputWorldGeometry->Clone().GetPointer());
        worldTimeSlicedGeometry = m_CreatedWorldGeometry.GetPointer();
        slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(m_CreatedWorldGeometry->GetGeometry3D(0));
        if(slicedWorldGeometry.IsNotNull())
        {
          break;
        }
      }
      else
      {
        const SlicedGeometry3D* worldSlicedGeometry = dynamic_cast<const SlicedGeometry3D*>(m_InputWorldGeometry.GetPointer());
        if(worldSlicedGeometry != NULL)
        {
          slicedWorldGeometry = static_cast<SlicedGeometry3D*>(m_InputWorldGeometry->Clone().GetPointer());
          break;
        }
      }
      //else: use Transversal: no "break" here!!
    case Transversal:
      slicedWorldGeometry=SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(m_InputWorldGeometry, PlaneGeometry::Transversal, top, frontside, rotated);
      break;
    case Frontal:
      slicedWorldGeometry=SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(m_InputWorldGeometry, PlaneGeometry::Frontal, top, frontside, rotated);
      break;
    case Sagittal:
      slicedWorldGeometry=SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(m_InputWorldGeometry, PlaneGeometry::Sagittal, top, frontside, rotated);
      break;
    default:
      itkExceptionMacro("unknown ViewDirection");
    }

    m_Slice->SetPos(0);

    m_Slice->SetSteps((int)slicedWorldGeometry->GetSlices());
  

  if(m_CreatedWorldGeometry.IsNull())
  {
    // initialize TimeSlicedGeometry
    m_CreatedWorldGeometry = TimeSlicedGeometry::New();
  }
  if(worldTimeSlicedGeometry==NULL)
  {
    m_CreatedWorldGeometry->InitializeEvenlyTimed(slicedWorldGeometry, 1);
    m_Time->SetSteps(0);
    m_Time->SetPos(0);
  }
  else
  {
    m_BlockUpdate = true;
    m_Time->SetSteps(worldTimeSlicedGeometry->GetTimeSteps());
    m_Time->SetPos(0);
    m_BlockUpdate = false;

    slicedWorldGeometry->SetTimeBounds(worldTimeSlicedGeometry->GetGeometry3D(0)->GetTimeBounds());
    //@todo implement for non-evenly-timed geometry!
    m_CreatedWorldGeometry->InitializeEvenlyTimed(slicedWorldGeometry, worldTimeSlicedGeometry->GetTimeSteps());
  }
}

  //unblock update; we may do this now, because if m_BlockUpdate was already true before this method was entered,
  //then we will never come here.
  m_BlockUpdate = false;

  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry and time/slice data.
  this->SendCreatedWorldGeometry();
  this->SendSlice();
  this->SendTime();
}

void SliceNavigationController::SendCreatedWorldGeometry()
{
  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry.
  if(!m_BlockUpdate)
    InvokeEvent(GeometrySendEvent(m_CreatedWorldGeometry, 0));
}

void SliceNavigationController::SendCreatedWorldGeometryUpdate()
{
  if(!m_BlockUpdate)
    InvokeEvent(GeometryUpdateEvent(m_CreatedWorldGeometry, m_Slice->GetPos()));
}

//##ModelId=3DD524D7038C
void SliceNavigationController::SendSlice()
{
  if(!m_BlockUpdate)
  {
    if(m_CreatedWorldGeometry.IsNotNull())
    {
      InvokeEvent(GeometrySliceEvent(m_CreatedWorldGeometry, m_Slice->GetPos()));

      // Request rendering update for all views
      GetRenderingManager()->RequestUpdateAll();
    }
  }
}

void SliceNavigationController::SendTime()
{
  if(!m_BlockUpdate)
  {
    if(m_CreatedWorldGeometry.IsNotNull())
    {
      InvokeEvent(GeometryTimeEvent(m_CreatedWorldGeometry, m_Time->GetPos()));

      // Request rendering update for all views
      GetRenderingManager()->RequestUpdateAll();
    }
  }
}

void SliceNavigationController::SetGeometry(const itk::EventObject&)
{
}

void SliceNavigationController::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
{
  const SliceNavigationController::GeometryTimeEvent * timeEvent =
    dynamic_cast<const SliceNavigationController::GeometryTimeEvent *>(&geometryTimeEvent);
  assert(timeEvent!=NULL);

  TimeSlicedGeometry* timeSlicedGeometry = timeEvent->GetTimeSlicedGeometry();
  assert(timeSlicedGeometry!=NULL);

  if(m_CreatedWorldGeometry.IsNotNull())
  {
    int timeStep = (int) timeEvent->GetPos();
    ScalarType timeInMS;
    timeInMS = timeSlicedGeometry->TimeStepToMS(timeStep);
    timeStep = m_CreatedWorldGeometry->MSToTimeStep(timeInMS);
    GetTime()->SetPos(timeStep);
  }
}

void SliceNavigationController::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(&geometrySliceEvent);
  assert(sliceEvent!=NULL);

  GetSlice()->SetPos(sliceEvent->GetPos());
}

void SliceNavigationController::ExecuteOperation(Operation* operation)
{
  // switch on type
  // - select best slice for a given point
  // - rotate created world geometry according to Operation->SomeInfo()
  if (!operation) return;
  
  switch ( operation->GetOperationType() )
  {
    case OpMOVE: // should be a point operation
    {
      if (m_SliceLocked) break; //do not move the cross position
      // select a slice
      PointOperation* po = dynamic_cast<PointOperation*>(operation);
      if (!po) return;

      Point3D point = po->GetPoint();
      //@todo add time to PositionEvent and use here!!
      SlicedGeometry3D* slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>( m_CreatedWorldGeometry->GetGeometry3D(0) );
      if ( slicedWorldGeometry )
      {
        int best_slice = -1;
        double best_distance = itk::NumericTraits<double>::max();

        int s, slices;
        slices = slicedWorldGeometry->GetSlices();
        if(slicedWorldGeometry->GetEvenlySpaced())
        {
          Point3D pointInUnits;
          slicedWorldGeometry->WorldToIndex(point, pointInUnits);
          best_slice = (int)(pointInUnits[2]+0.5);
        }
        else
        {
          Point3D projected_point;
          for( s=0; s < slices; ++s )
          {
            slicedWorldGeometry->GetGeometry2D(s)->Project(point, projected_point);
            Vector3D dist = projected_point - point;
            ScalarType curDist = dist.GetSquaredNorm();
            if(curDist < best_distance)
            {
              best_distance = curDist;
              best_slice    = s;
            }
          }
        }
        if(best_slice >= 0)
          GetSlice()->SetPos(best_slice);
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

/// Relict from the old times, when automous descisions were accepted behaviour.
/// Remains in here, because some RenderWindows do exist outide of StdMultiWidgets.
bool SliceNavigationController::ExecuteAction( Action* action, StateEvent const* stateEvent)
{
  bool ok = false;

  const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if(posEvent!=NULL)
  {
    if(m_CreatedWorldGeometry.IsNull())
      return true;
    switch (action->GetActionId())
    {
    case AcMOVE:
      {
        BaseRenderer* baseRenderer = posEvent->GetSender();
        if (!baseRenderer)
        {
          baseRenderer = const_cast<BaseRenderer *>( GlobalInteraction::GetInstance()->GetFocus() );
        }
        if (baseRenderer)
          if (baseRenderer->GetMapperID() == 1)
          {
            PointOperation po( OpMOVE, posEvent->GetWorldPosition() );

            ExecuteOperation( &po ); // select best slice
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

  const DisplayPositionEvent* displPosEvent = dynamic_cast<const DisplayPositionEvent *>(stateEvent->GetEvent());
  if(displPosEvent!=NULL)
  {
    return true;
  }

  return false;
}

} // namespace

