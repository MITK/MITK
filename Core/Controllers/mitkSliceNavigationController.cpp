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
#include <itkCommand.h>
#include <mitkGlobalInteraction.h>
#include <mitkEventMapper.h>
#include <mitkFocusManager.h>
#include <mitkOpenGLRenderer.h>

//##ModelId=3E189B1D008D
mitk::SliceNavigationController::SliceNavigationController(const char * type) 
  : BaseController(type), m_ViewDirection(Transversal), m_BlockUpdate(false), m_CreatedWorldGeometry(NULL), m_InputWorldGeometry(NULL)
{
  itk::SimpleMemberCommand<SliceNavigationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  timeStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
#ifdef WIN32
  sliceStepperChangedCommand->SetCallbackFunction(this, SliceNavigationController::SendSlice);
  timeStepperChangedCommand->SetCallbackFunction(this,  SliceNavigationController::SendTime);
#else
  sliceStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SendSlice);
  timeStepperChangedCommand->SetCallbackFunction(this,  &SliceNavigationController::SendTime);
#endif

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
  if(m_BlockUpdate)
    return;
  m_BlockUpdate = true;
  if(m_LastUpdateTime<GetMTime())
  {
    m_LastUpdateTime = GetMTime();

    if(m_InputWorldGeometry.IsNotNull())
    {
      // initialize the viewplane
      mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

      switch(m_ViewDirection)
      {
        case Original:
          assert(false);
          break;
        case Transversal:
          planegeometry->InitializeStandardPlane(m_InputWorldGeometry, PlaneGeometry::Transversal, m_InputWorldGeometry->GetExtentInMM(2), false);
          m_Slice->SetSteps((int)m_InputWorldGeometry->GetExtentInMM(2)+1.0);
          break;
        case Frontal:
          planegeometry->InitializeStandardPlane(m_InputWorldGeometry, PlaneGeometry::Frontal);
          m_Slice->SetSteps((int)m_InputWorldGeometry->GetExtentInMM(1)+1.0);
          break;
        case Sagittal:
          planegeometry->InitializeStandardPlane(m_InputWorldGeometry, PlaneGeometry::Sagittal);
          m_Slice->SetSteps((int)m_InputWorldGeometry->GetExtentInMM(0)+1.0);
          break;
        default:
          itkExceptionMacro("unknown ViewDirection");
      }

      m_Slice->SetPos(0);

      mitk::SlicedGeometry3D::Pointer slicedWorldGeometry=mitk::SlicedGeometry3D::New();
      slicedWorldGeometry->InitializeEvenlySpaced(planegeometry, 1, m_Slice->GetSteps(), (m_ViewDirection==Frontal?true:false));

      // initialize TimeSlicedGeometry
      m_CreatedWorldGeometry = TimeSlicedGeometry::New();
      const TimeSlicedGeometry* worldTimeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(m_InputWorldGeometry.GetPointer());
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

        slicedWorldGeometry->SetTimeBoundsInMS(worldTimeSlicedGeometry->GetGeometry3D(0)->GetTimeBoundsInMS());
        //@todo implement for non-evenly-timed geometry!
        m_CreatedWorldGeometry->InitializeEvenlyTimed(slicedWorldGeometry, worldTimeSlicedGeometry->GetTimeSteps());
      }
    }
  }

  //unblock update; we may do this now, because if m_BlockUpdate was already true before this method was entered,
  //then we will never come here.
  m_BlockUpdate = false;

  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry and time/slice data.
  SendCreatedWorldGeometry();
  SendSlice();
  SendTime();
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
      InvokeEvent(UpdateEvent());
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
      InvokeEvent(UpdateEvent());
    }
  }
}

void mitk::SliceNavigationController::SetGeometry(const itk::EventObject & geometrySendEvent)
{
}

void mitk::SliceNavigationController::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
{
}

void mitk::SliceNavigationController::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
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
            mitk::Point3D point, projected_point; 
            point = posEvent->GetWorldPosition();
            
            //@todo add time to PositionEvent and use here!!
            SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_CreatedWorldGeometry->GetGeometry3D(0));
            if(slicedWorldGeometry!=NULL)
            {
              int best_slice = -1;
              double best_distance = itk::NumericTraits<double>::max();

              int s, slices;
              slices = slicedWorldGeometry->GetSlices();
              for(s=0; s < slices; ++s)
              {
                slicedWorldGeometry->GetGeometry2D(s)->Project(point, projected_point);
                Vector3D dist = projected_point-point;
                if(dist.GetSquaredNorm() < best_distance)
                {
                  best_distance = dist.GetSquaredNorm();
                  best_slice    = s;
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
