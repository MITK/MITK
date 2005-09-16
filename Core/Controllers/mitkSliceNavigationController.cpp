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


#include "mitkSliceNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkPositionEvent.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkGlobalInteraction.h"
#include "mitkEventMapper.h"
#include "mitkFocusManager.h"
#include "mitkOpenGLRenderer.h"
#include "mitkRenderingManager.h"

#include <itkCommand.h>

//##ModelId=3E189B1D008D
mitk::SliceNavigationController::SliceNavigationController(const char * type) 
  : BaseController(type), m_InputWorldGeometry(NULL), m_CreatedWorldGeometry(NULL), m_ViewDirection(Transversal), m_BlockUpdate(false)
{
  itk::SimpleMemberCommand<SliceNavigationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  timeStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  sliceStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SendSlice);
  timeStepperChangedCommand->SetCallbackFunction(this,  &SliceNavigationController::SendTime);

  m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
  m_Time->AddObserver(itk::ModifiedEvent(),  timeStepperChangedCommand);
  ConnectGeometryEvents(this);
}

//##ModelId=3E189B1D00BF
mitk::SliceNavigationController::~SliceNavigationController()
{

}

void mitk::SliceNavigationController::SetInputWorldGeometry(const mitk::Geometry3D* geometry)
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

void mitk::SliceNavigationController::Update()
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
  if(m_LastUpdateTime < GetMTime())
  {
    m_LastUpdateTime = GetMTime();

    // initialize the viewplane
    mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

    ScalarType viewSpacing = 1;

    mitk::SlicedGeometry3D::Pointer slicedWorldGeometry = NULL;
    m_CreatedWorldGeometry = NULL;
    const TimeSlicedGeometry* worldTimeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(m_InputWorldGeometry.GetPointer());
    switch(m_ViewDirection)
    {
      case Original:
        if(worldTimeSlicedGeometry != NULL)
        {
          m_CreatedWorldGeometry =
            static_cast<mitk::TimeSlicedGeometry*>(m_InputWorldGeometry->Clone().GetPointer());
          worldTimeSlicedGeometry = m_CreatedWorldGeometry.GetPointer();
          slicedWorldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(m_CreatedWorldGeometry->GetGeometry3D(0));
          if(slicedWorldGeometry.IsNotNull())
          {
            break;
          }
        }
        else
        {
          const mitk::SlicedGeometry3D* worldSlicedGeometry = dynamic_cast<const mitk::SlicedGeometry3D*>(m_InputWorldGeometry.GetPointer());
          if(worldSlicedGeometry != NULL)
          {
            slicedWorldGeometry = static_cast<mitk::SlicedGeometry3D*>(m_InputWorldGeometry->Clone().GetPointer());
            break;
          }
        }
        //else: use Transversal: no "break" here!!
      case Transversal:
        planegeometry->InitializeStandardPlane(            
          m_InputWorldGeometry,
          PlaneGeometry::Transversal, 
          m_InputWorldGeometry->GetExtent(2)-1+0.5, false);
        m_Slice->SetSteps((int)m_InputWorldGeometry->GetExtent(2));
        viewSpacing=m_InputWorldGeometry->GetExtentInMM(2)/m_InputWorldGeometry->GetExtent(2);
        break;
      case Frontal:
        planegeometry->InitializeStandardPlane(
          m_InputWorldGeometry,
          PlaneGeometry::Frontal, +0.5);
        m_Slice->SetSteps((int)(m_InputWorldGeometry->GetExtent(1)));
        viewSpacing=m_InputWorldGeometry->GetExtentInMM(1)/m_InputWorldGeometry->GetExtent(1);
        break;
      case Sagittal:
        planegeometry->InitializeStandardPlane(
          m_InputWorldGeometry,
          PlaneGeometry::Sagittal, +0.5);
        m_Slice->SetSteps((int)(m_InputWorldGeometry->GetExtent(0)));
        viewSpacing=m_InputWorldGeometry->GetExtentInMM(0)/m_InputWorldGeometry->GetExtent(0);
        break;
      default:
        itkExceptionMacro("unknown ViewDirection");
    }

    m_Slice->SetPos(0);

    if(slicedWorldGeometry.IsNull())
    {
      slicedWorldGeometry=mitk::SlicedGeometry3D::New();
      slicedWorldGeometry->InitializeEvenlySpaced(planegeometry, viewSpacing, m_Slice->GetSteps(), (m_ViewDirection==Frontal?true:false));
    }
    else
    {
      m_Slice->SetSteps((int)slicedWorldGeometry->GetSlices());
    }

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

void mitk::SliceNavigationController::SendCreatedWorldGeometry()
{
  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry.
  if(!m_BlockUpdate)
    InvokeEvent(GeometrySendEvent(m_CreatedWorldGeometry, 0));
}

//##ModelId=3DD524D7038C
void mitk::SliceNavigationController::SendSlice()
{
  if(!m_BlockUpdate)
  {
    if(m_CreatedWorldGeometry.IsNotNull())
    {
      InvokeEvent(GeometrySliceEvent(m_CreatedWorldGeometry, m_Slice->GetPos()));

      // Request rendering update for all views
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void mitk::SliceNavigationController::SendTime()
{
  if(!m_BlockUpdate)
  {
    if(m_CreatedWorldGeometry.IsNotNull())
    {
      InvokeEvent(GeometryTimeEvent(m_CreatedWorldGeometry, m_Time->GetPos()));

      // Request rendering update for all views
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void mitk::SliceNavigationController::SetGeometry(const itk::EventObject & geometrySendEvent)
{
}

void mitk::SliceNavigationController::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
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

void mitk::SliceNavigationController::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(&geometrySliceEvent);
  assert(sliceEvent!=NULL);

  GetSlice()->SetPos(sliceEvent->GetPos());
}

bool mitk::SliceNavigationController::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent)
{
    bool ok = false;
    //if (m_Destination == NULL)
    //    return false;
	
    const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if(posEvent!=NULL)
    {
      if(m_CreatedWorldGeometry.IsNull())
        return true;
      switch (action->GetActionId())
      {
        case AcMOVEPOINT:
        {
          mitk::BaseRenderer* baseRenderer = posEvent->GetSender();
          if (baseRenderer==NULL)
            {
              mitk::GlobalInteraction *globalInteraction = dynamic_cast<mitk::GlobalInteraction *>(mitk::EventMapper::GetGlobalStateMachine());
              if (globalInteraction!=NULL)
              {
                baseRenderer = const_cast<mitk::BaseRenderer *>(globalInteraction->GetFocus());
              }
            }
          if (baseRenderer!=NULL)
           if (baseRenderer->GetMapperID() == 1)
           {
            mitk::Point3D point;
            point = posEvent->GetWorldPosition();
            
            //@todo add time to PositionEvent and use here!!
            SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_CreatedWorldGeometry->GetGeometry3D(0));
            if(slicedWorldGeometry!=NULL)
            {
              int best_slice = -1;
              double best_distance = itk::NumericTraits<double>::max();

              int s, slices;
              slices = slicedWorldGeometry->GetSlices();
              if(slicedWorldGeometry->GetEvenlySpaced())
              {
                mitk::Point3D pointInUnits;
                slicedWorldGeometry->WorldToIndex(point, pointInUnits);
                best_slice = (int)(pointInUnits[2]+0.5);
              }
              else
              {
                mitk::Point3D projected_point;
                for(s=0; s < slices; ++s)
                {
                  slicedWorldGeometry->GetGeometry2D(s)->Project(point, projected_point);
                  Vector3D dist = projected_point-point;
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
          }
          ok = true;
          break;
        }
        case AcFINISHMOVEMENT:
        {
       // /*finishes a Movement from the coordinate supplier: 
       //   gets the lastpoint from the undolist and writes an undo-operation so 
       //   that the movement of the coordinatesupplier is undoable.*/
       //   mitk::Point3D movePoint, oldMovePoint;
       //   oldMovePoint.Fill(0);
       //   vm2itk(posEvent->GetWorldPosition(), movePoint);
       //   PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
       //   if (m_UndoEnabled )
       //   {
       //     //get the last Position from the UndoList
       //     OperationEvent *lastOperationEvent = m_UndoController->GetLastOfType(m_Destination, OpMOVE);
       //     if (lastOperationEvent != NULL)
       //     {
       //       PointOperation* lastOp = dynamic_cast<PointOperation *>(lastOperationEvent->GetOperation());
       //       if (lastOp != NULL)
       //       {
       //         oldMovePoint = lastOp->GetPoint();
       //       }
       //     }
       //     PointOperation* undoOp = new PointOperation(OpMOVE, oldMovePoint, 0);
       //     OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp);
       //     m_UndoController->SetOperationEvent(operationEvent);
       //   }
       //   //execute the Operation
	  		  //m_Destination->ExecuteOperation(doOp);
         
          //mitk::BaseRenderer *baseRenderer = posEvent->GetSender();
          //if (baseRenderer==NULL)
          //  {
          //    mitk::GlobalInteraction *globalInteraction = dynamic_cast<mitk::GlobalInteraction *>(mitk::EventMapper::GetGlobalStateMachine());
          //    if (globalInteraction!=NULL)
          //    {
          //      baseRenderer = const_cast<mitk::BaseRenderer *>(globalInteraction->GetFocus());
          //    }
          //  }
          //  if (baseRenderer!=NULL)
          //  {
          //    if (baseRenderer->GetMapperID() == 1)
          //    {
          //      mitk::Point3D point, projected_point; 
          //      point = posEvent->GetWorldPosition();
          //      
          //      //@todo add time to PositionEvent and use here!!
          //      SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_CreatedWorldGeometry->GetGeometry3D(0));
          //      if(slicedWorldGeometry!=NULL)
          //      {
          //        int best_slice = -1;
          //        double best_distance = itk::NumericTraits<double>::max();

          //        int s, slices;
          //        slices = slicedWorldGeometry->GetSlices();
          //        for(s=0; s < slices; ++s)
          //        {
          //          slicedWorldGeometry->GetGeometry2D(s)->Project(point, projected_point);
          //          Vector3D dist = projected_point-point;
          //          if(dist.lengthSquared() < best_distance)
          //          {
          //            best_distance = dist.lengthSquared();
          //            best_slice    = s;
          //          }
          //        }
          //        if(best_slice >= 0)
          //          GetSlice()->SetPos(best_slice);
          //      }
          //    }
          //  }
            ok = true;
            break;
          }
        default:
          ok = true;
          break;
        }
       return ok;
    }

    const mitk::DisplayPositionEvent* displPosEvent = dynamic_cast<const mitk::DisplayPositionEvent *>(stateEvent->GetEvent());
    if(displPosEvent!=NULL)
    {
        return true;
    }

	return false;
}
