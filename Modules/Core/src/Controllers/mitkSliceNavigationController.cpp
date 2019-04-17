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

#include "mitkSliceNavigationController.h"
#include "mitkAction.h"
#include "mitkBaseRenderer.h"
#include "mitkCrosshairPositionEvent.h"
#include "mitkInteractionConst.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkPlaneGeometry.h"
#include "mitkProportionalTimeGeometry.h"
#include "mitkArbitraryTimeGeometry.h"
#include "mitkRenderingManager.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkVtkPropRenderer.h"

#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkInteractionConst.h"
#include "mitkNodePredicateDataType.h"
#include "mitkOperationEvent.h"
#include "mitkPixelTypeMultiplex.h"
#include "mitkPlaneOperation.h"
#include "mitkPointOperation.h"
#include "mitkStatusBar.h"
#include "mitkUndoController.h"

#include "mitkApplyTransformMatrixOperation.h"

#include "mitkMemoryUtilities.h"

#include <itkCommand.h>

namespace mitk
{
  SliceNavigationController::SliceNavigationController()
    : BaseController(),
  m_InputWorldGeometry3D( mitk::BaseGeometry::ConstPointer() ),
  m_InputWorldTimeGeometry( mitk::TimeGeometry::ConstPointer() ),
  m_CreatedWorldGeometry( mitk::TimeGeometry::Pointer() ),
      m_ViewDirection(Axial),
      m_DefaultViewDirection(Axial),
  m_RenderingManager( mitk::RenderingManager::Pointer() ),
  m_Renderer( nullptr ),
      m_Top(false),
      m_FrontSide(false),
      m_Rotated(false),
      m_BlockUpdate(false),
      m_SliceLocked(false),
      m_SliceRotationLocked(false),
      m_OldPos(0)
  {
    typedef itk::SimpleMemberCommand<SliceNavigationController> SNCCommandType;
    SNCCommandType::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;

    sliceStepperChangedCommand = SNCCommandType::New();
    timeStepperChangedCommand = SNCCommandType::New();

    sliceStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SendSlice);

    timeStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SendTime);

    m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
    m_Time->AddObserver(itk::ModifiedEvent(), timeStepperChangedCommand);

    m_Slice->SetUnitName("mm");
    m_Time->SetUnitName("ms");

    m_Top = false;
    m_FrontSide = false;
    m_Rotated = false;
  }

  SliceNavigationController::~SliceNavigationController() {}
  void SliceNavigationController::SetInputWorldGeometry3D(const BaseGeometry *geometry)
  {
  if ( geometry != nullptr )
    {
      if (geometry->GetBoundingBox()->GetDiagonalLength2() < eps)
      {
        itkWarningMacro("setting an empty bounding-box");
      geometry = nullptr;
      }
    }
    if (m_InputWorldGeometry3D != geometry)
    {
      m_InputWorldGeometry3D = geometry;
    m_InputWorldTimeGeometry = mitk::TimeGeometry::ConstPointer();
      this->Modified();
    }
  }

  void SliceNavigationController::SetInputWorldTimeGeometry(const TimeGeometry *geometry)
  {
  if ( geometry != nullptr )
    {
      if (geometry->GetBoundingBoxInWorld()->GetDiagonalLength2() < eps)
      {
        itkWarningMacro("setting an empty bounding-box");
        geometry = nullptr;
      }
    }
    if (m_InputWorldTimeGeometry != geometry)
    {
      m_InputWorldTimeGeometry = geometry;
      m_InputWorldGeometry3D = mitk::BaseGeometry::ConstPointer();
      this->Modified();
    }
  }

  RenderingManager *SliceNavigationController::GetRenderingManager() const
  {
    mitk::RenderingManager *renderingManager = m_RenderingManager.GetPointer();

    if (renderingManager != nullptr)
      return renderingManager;

    if ( m_Renderer != nullptr )
    {
      renderingManager = m_Renderer->GetRenderingManager();

      if (renderingManager != nullptr)
        return renderingManager;
    }

    return mitk::RenderingManager::GetInstance();
  }

  void SliceNavigationController::SetViewDirectionToDefault() { m_ViewDirection = m_DefaultViewDirection; }
  const char *SliceNavigationController::GetViewDirectionAsString() const
  {
    const char *viewDirectionString;
    switch (m_ViewDirection)
    {
      case SliceNavigationController::Axial:
        viewDirectionString = "Axial";
        break;

      case SliceNavigationController::Sagittal:
        viewDirectionString = "Sagittal";
        break;

      case SliceNavigationController::Frontal:
        viewDirectionString = "Coronal";
        break;

      case SliceNavigationController::Original:
        viewDirectionString = "Original";
        break;

      default:
        viewDirectionString = "No View Direction Available";
        break;
    }
    return viewDirectionString;
  }

  void SliceNavigationController::Update()
  {
    if (!m_BlockUpdate)
    {
      if (m_ViewDirection == Sagittal)
      {
        this->Update(Sagittal, true, true, false);
      }
      else if (m_ViewDirection == Frontal)
      {
        this->Update(Frontal, false, true, false);
      }
      else if (m_ViewDirection == Axial)
      {
        this->Update(Axial, false, false, true);
      }
      else
      {
        this->Update(m_ViewDirection);
      }
    }
  }

  void SliceNavigationController::Update(SliceNavigationController::ViewDirection viewDirection,
                                         bool top,
                                         bool frontside,
                                         bool rotated)
  {
    TimeGeometry::ConstPointer worldTimeGeometry = m_InputWorldTimeGeometry;

    if (m_BlockUpdate || (m_InputWorldTimeGeometry.IsNull() && m_InputWorldGeometry3D.IsNull()) ||
        ((worldTimeGeometry.IsNotNull()) && (worldTimeGeometry->CountTimeSteps() == 0)))
    {
      return;
    }

    m_BlockUpdate = true;

    if (m_InputWorldTimeGeometry.IsNotNull() && m_LastUpdateTime < m_InputWorldTimeGeometry->GetMTime())
    {
      Modified();
    }
    if (m_InputWorldGeometry3D.IsNotNull() && m_LastUpdateTime < m_InputWorldGeometry3D->GetMTime())
    {
      Modified();
    }
    this->SetViewDirection(viewDirection);
    this->SetTop(top);
    this->SetFrontSide(frontside);
    this->SetRotated(rotated);

    if (m_LastUpdateTime < GetMTime())
    {
      m_LastUpdateTime = GetMTime();

      // initialize the viewplane
      SlicedGeometry3D::Pointer slicedWorldGeometry = SlicedGeometry3D::Pointer();
      BaseGeometry::ConstPointer currentGeometry = BaseGeometry::ConstPointer();
      if (m_InputWorldTimeGeometry.IsNotNull())
        if (m_InputWorldTimeGeometry->IsValidTimeStep(GetTime()->GetPos()))
          currentGeometry = m_InputWorldTimeGeometry->GetGeometryForTimeStep(GetTime()->GetPos());
        else
          currentGeometry = m_InputWorldTimeGeometry->GetGeometryForTimeStep(0);
      else
        currentGeometry = m_InputWorldGeometry3D;

      m_CreatedWorldGeometry = mitk::TimeGeometry::Pointer();
      switch (viewDirection)
      {
        case Original:
          if (worldTimeGeometry.IsNotNull())
          {
            m_CreatedWorldGeometry = worldTimeGeometry->Clone();

            worldTimeGeometry = m_CreatedWorldGeometry.GetPointer();

            slicedWorldGeometry = dynamic_cast<SlicedGeometry3D *>(
              m_CreatedWorldGeometry->GetGeometryForTimeStep(this->GetTime()->GetPos()).GetPointer());

            if (slicedWorldGeometry.IsNotNull())
            {
              break;
            }
          }
          else
          {
            const auto *worldSlicedGeometry =
              dynamic_cast<const SlicedGeometry3D *>(currentGeometry.GetPointer());

          if ( worldSlicedGeometry != nullptr )
            {
              slicedWorldGeometry = static_cast<SlicedGeometry3D *>(currentGeometry->Clone().GetPointer());
              break;
            }
          }
          slicedWorldGeometry = SlicedGeometry3D::New();
          slicedWorldGeometry->InitializePlanes(currentGeometry, PlaneGeometry::None, top, frontside, rotated);
          slicedWorldGeometry->SetSliceNavigationController(this);
          break;

        case Axial:
          slicedWorldGeometry = SlicedGeometry3D::New();
          slicedWorldGeometry->InitializePlanes(currentGeometry, PlaneGeometry::Axial, top, frontside, rotated);
          slicedWorldGeometry->SetSliceNavigationController(this);
          break;

        case Frontal:
          slicedWorldGeometry = SlicedGeometry3D::New();
          slicedWorldGeometry->InitializePlanes(currentGeometry, PlaneGeometry::Frontal, top, frontside, rotated);
          slicedWorldGeometry->SetSliceNavigationController(this);
          break;

        case Sagittal:
          slicedWorldGeometry = SlicedGeometry3D::New();
          slicedWorldGeometry->InitializePlanes(currentGeometry, PlaneGeometry::Sagittal, top, frontside, rotated);
          slicedWorldGeometry->SetSliceNavigationController(this);
          break;
        default:
          itkExceptionMacro("unknown ViewDirection");
      }

      m_Slice->SetPos(0);
      m_Slice->SetSteps((int)slicedWorldGeometry->GetSlices());

      if ( worldTimeGeometry.IsNull() )
      {
        auto createdTimeGeometry = ProportionalTimeGeometry::New();
        createdTimeGeometry->Initialize( slicedWorldGeometry, 1 );
        m_CreatedWorldGeometry = createdTimeGeometry;
      
        m_Time->SetSteps(0);
        m_Time->SetPos(0);
        m_Time->InvalidateRange();
      }
      else
      {
        m_BlockUpdate = true;
        m_Time->SetSteps(worldTimeGeometry->CountTimeSteps());
        m_Time->SetPos(0);

        const TimeBounds &timeBounds = worldTimeGeometry->GetTimeBounds();
        m_Time->SetRange(timeBounds[0], timeBounds[1]);

        m_BlockUpdate = false;

        const auto currentTemporalPosition = this->GetTime()->GetPos();
        assert( worldTimeGeometry->GetGeometryForTimeStep( currentTemporalPosition ).IsNotNull() );

      if ( dynamic_cast<const mitk::ProportionalTimeGeometry*>( worldTimeGeometry.GetPointer() ) != nullptr )
      {
        const TimePointType minimumTimePoint =
          worldTimeGeometry->TimeStepToTimePoint( currentTemporalPosition );
        
        const TimePointType stepDuration =
          worldTimeGeometry->TimeStepToTimePoint( currentTemporalPosition + 1 ) - minimumTimePoint;

        auto createdTimeGeometry = ProportionalTimeGeometry::New();
        createdTimeGeometry->Initialize( slicedWorldGeometry, worldTimeGeometry->CountTimeSteps() );
        createdTimeGeometry->SetFirstTimePoint( minimumTimePoint );
        createdTimeGeometry->SetStepDuration( stepDuration );

        m_CreatedWorldGeometry = createdTimeGeometry;
      }
      else
      {
        auto createdTimeGeometry = mitk::ArbitraryTimeGeometry::New();
        const TimeStepType numberOfTimeSteps = worldTimeGeometry->CountTimeSteps();
        createdTimeGeometry->ReserveSpaceForGeometries( numberOfTimeSteps );

        for ( TimeStepType i = 0; i < numberOfTimeSteps; ++i )
        {
          const BaseGeometry::Pointer clonedGeometry = slicedWorldGeometry->Clone().GetPointer();
          const auto bounds = worldTimeGeometry->GetTimeBounds( i );
          createdTimeGeometry->AppendNewTimeStep( clonedGeometry,
            bounds[0], bounds[1]);
        }
        createdTimeGeometry->Update();

        m_CreatedWorldGeometry = createdTimeGeometry;
      }
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

  void SliceNavigationController::SendCreatedWorldGeometry()
  {
    // Send the geometry. Do this even if nothing was changed, because maybe
    // Update() was only called to re-send the old geometry.
    if (!m_BlockUpdate)
    {
      this->InvokeEvent(GeometrySendEvent(m_CreatedWorldGeometry, 0));
    }
  }

  void SliceNavigationController::SendCreatedWorldGeometryUpdate()
  {
    if (!m_BlockUpdate)
    {
      this->InvokeEvent(GeometryUpdateEvent(m_CreatedWorldGeometry, m_Slice->GetPos()));
    }
  }

  void SliceNavigationController::SendSlice()
  {
    if (!m_BlockUpdate)
    {
      if (m_CreatedWorldGeometry.IsNotNull())
      {
        this->InvokeEvent(GeometrySliceEvent(m_CreatedWorldGeometry, m_Slice->GetPos()));

        // Request rendering update for all views
        this->GetRenderingManager()->RequestUpdateAll();
      }
    }
  }

  void SliceNavigationController::SendTime()
  {
    if (!m_BlockUpdate)
    {
      if (m_CreatedWorldGeometry.IsNotNull())
      {
        this->InvokeEvent(GeometryTimeEvent(m_CreatedWorldGeometry, m_Time->GetPos()));

        // Request rendering update for all views
        this->GetRenderingManager()->RequestUpdateAll();
      }
    }
  }

  void SliceNavigationController::SetGeometry(const itk::EventObject &) {}
  void SliceNavigationController::SetGeometryTime(const itk::EventObject &geometryTimeEvent)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    const auto *timeEvent =
      dynamic_cast< const SliceNavigationController::GeometryTimeEvent * >(&geometryTimeEvent);
    assert( timeEvent != nullptr );

    TimeGeometry *timeGeometry = timeEvent->GetTimeGeometry();
    assert( timeGeometry != nullptr );

    auto timeStep = (int)timeEvent->GetPos();
    ScalarType timeInMS;
    timeInMS = timeGeometry->TimeStepToTimePoint(timeStep);
    timeStep = m_CreatedWorldGeometry->TimePointToTimeStep(timeInMS);
    this->GetTime()->SetPos(timeStep);
  }

  void SliceNavigationController::SetGeometrySlice(const itk::EventObject &geometrySliceEvent)
  {
    const auto *sliceEvent =
      dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(&geometrySliceEvent);
    assert(sliceEvent!=nullptr);

    this->GetSlice()->SetPos(sliceEvent->GetPos());
  }

  void SliceNavigationController::SelectSliceByPoint(const Point3D &point)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    //@todo add time to PositionEvent and use here!!
    SlicedGeometry3D *slicedWorldGeometry = dynamic_cast<SlicedGeometry3D *>(
      m_CreatedWorldGeometry->GetGeometryForTimeStep(this->GetTime()->GetPos()).GetPointer());

    if (slicedWorldGeometry)
    {
      int bestSlice = -1;
      double bestDistance = itk::NumericTraits<double>::max();

      int s, slices;
      slices = slicedWorldGeometry->GetSlices();
      if (slicedWorldGeometry->GetEvenlySpaced())
      {
        mitk::PlaneGeometry *plane = slicedWorldGeometry->GetPlaneGeometry(0);

        const Vector3D &direction = slicedWorldGeometry->GetDirectionVector();

        Point3D projectedPoint;
        plane->Project(point, projectedPoint);

        // Check whether the point is somewhere within the slice stack volume;
        // otherwise, the default slice (0) will be selected
        if (direction[0] * (point[0] - projectedPoint[0]) + direction[1] * (point[1] - projectedPoint[1]) +
              direction[2] * (point[2] - projectedPoint[2]) >=
            0)
        {
          bestSlice = (int)(plane->Distance(point) / slicedWorldGeometry->GetSpacing()[2] + 0.5);
        }
      }
      else
      {
        Point3D projectedPoint;
        for (s = 0; s < slices; ++s)
        {
          slicedWorldGeometry->GetPlaneGeometry(s)->Project(point, projectedPoint);
          const Vector3D distance = projectedPoint - point;
          ScalarType currentDistance = distance.GetSquaredNorm();

          if (currentDistance < bestDistance)
          {
            bestDistance = currentDistance;
            bestSlice = s;
          }
        }
      }
      if (bestSlice >= 0)
      {
        this->GetSlice()->SetPos(bestSlice);
      }
      else
      {
        this->GetSlice()->SetPos(0);
      }
      this->SendCreatedWorldGeometryUpdate();
      // send crosshair event
      SetCrosshairEvent.Send(point);
    }
  }

  void SliceNavigationController::ReorientSlices(const Point3D &point, const Vector3D &normal)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    PlaneOperation op(OpORIENT, point, normal);

    m_CreatedWorldGeometry->ExecuteOperation(&op);

    this->SendCreatedWorldGeometryUpdate();
  }

  void SliceNavigationController::ReorientSlices(const mitk::Point3D &point,
                                                 const mitk::Vector3D &axisVec0,
                                                 const mitk::Vector3D &axisVec1)
  {
    if (m_CreatedWorldGeometry)
    {
      PlaneOperation op(OpORIENT, point, axisVec0, axisVec1);
      m_CreatedWorldGeometry->ExecuteOperation(&op);

      this->SendCreatedWorldGeometryUpdate();
    }
  }

  mitk::TimeGeometry *SliceNavigationController::GetCreatedWorldGeometry() { return m_CreatedWorldGeometry; }
  const mitk::BaseGeometry *SliceNavigationController::GetCurrentGeometry3D()
  {
    if (m_CreatedWorldGeometry.IsNotNull())
    {
      return m_CreatedWorldGeometry->GetGeometryForTimeStep(this->GetTime()->GetPos());
    }
    else
    {
      return nullptr;
    }
  }

  const mitk::PlaneGeometry *SliceNavigationController::GetCurrentPlaneGeometry()
  {
    const auto *slicedGeometry =
      dynamic_cast<const mitk::SlicedGeometry3D *>(this->GetCurrentGeometry3D());

    if (slicedGeometry)
    {
      const mitk::PlaneGeometry *planeGeometry = (slicedGeometry->GetPlaneGeometry(this->GetSlice()->GetPos()));
      return planeGeometry;
    }
    else
    {
      return nullptr;
    }
  }

  void SliceNavigationController::SetRenderer(BaseRenderer *renderer) { m_Renderer = renderer; }
  BaseRenderer *SliceNavigationController::GetRenderer() const { return m_Renderer; }
  void SliceNavigationController::AdjustSliceStepperRange()
  {
    const auto *slicedGeometry =
      dynamic_cast<const mitk::SlicedGeometry3D *>(this->GetCurrentGeometry3D());

    const Vector3D &direction = slicedGeometry->GetDirectionVector();

    int c = 0;
    int i, k = 0;
    for (i = 0; i < 3; ++i)
    {
      if (fabs(direction[i]) < 0.000000001)
      {
        ++c;
      }
      else
      {
        k = i;
      }
    }

    if (c == 2)
    {
      ScalarType min = slicedGeometry->GetOrigin()[k];
      ScalarType max = min + slicedGeometry->GetExtentInMM(k);

      m_Slice->SetRange(min, max);
    }
    else
    {
      m_Slice->InvalidateRange();
    }
  }

  void SliceNavigationController::ExecuteOperation(Operation *operation)
  {
    // switch on type
    // - select best slice for a given point
    // - rotate created world geometry according to Operation->SomeInfo()
    if (!operation || m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    switch (operation->GetOperationType())
    {
      case OpMOVE: // should be a point operation
      {
        if (!m_SliceLocked) // do not move the cross position
        {
          // select a slice
          auto *po = dynamic_cast<PointOperation *>(operation);
          if (po && po->GetIndex() == -1)
          {
            this->SelectSliceByPoint(po->GetPoint());
          }
          else if (po &&
                   po->GetIndex() != -1) // undo case because index != -1, index holds the old position of this slice
          {
            this->GetSlice()->SetPos(po->GetIndex());
          }
        }
        break;
      }
      case OpRESTOREPLANEPOSITION:
      {
        m_CreatedWorldGeometry->ExecuteOperation(operation);

        this->SendCreatedWorldGeometryUpdate();

        break;
      }
      case OpAPPLYTRANSFORMMATRIX:
      {
        m_CreatedWorldGeometry->ExecuteOperation(operation);

        this->SendCreatedWorldGeometryUpdate();

        break;
      }
      default:
      {
        // do nothing
        break;
      }
    }
  }

} // namespace
