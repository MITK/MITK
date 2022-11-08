/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSliceNavigationController.h"

#include <mitkSliceNavigationHelper.h>
#include "mitkBaseRenderer.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"

#include "mitkProportionalTimeGeometry.h"
#include "mitkArbitraryTimeGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkVtkPropRenderer.h"

#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkInteractionConst.h"
#include "mitkNodePredicateDataType.h"
#include "mitkOperationEvent.h"
#include "mitkPixelTypeMultiplex.h"
#include "mitkPlaneGeometry.h"
#include "mitkPlaneOperation.h"
#include "mitkPointOperation.h"

#include "mitkApplyTransformMatrixOperation.h"

namespace mitk
{
  SliceNavigationController::SliceNavigationController()
    : BaseController()
    , m_InputWorldTimeGeometry(TimeGeometry::ConstPointer())
    , m_CreatedWorldGeometry(TimeGeometry::Pointer())
    , m_ViewDirection(AnatomicalPlane::Axial)
    , m_DefaultViewDirection(AnatomicalPlane::Axial)
    , m_RenderingManager(RenderingManager::Pointer())
    , m_Renderer(nullptr)
    , m_BlockUpdate(false)
    , m_SliceLocked(false)
    , m_SliceRotationLocked(false)
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
  }

  SliceNavigationController::~SliceNavigationController()
  {
    // nothing here
  }

  void SliceNavigationController::SetInputWorldTimeGeometry(const TimeGeometry* geometry)
  {
    if (nullptr != geometry)
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
      this->Modified();
    }
  }

  void SliceNavigationController::SetViewDirectionToDefault()
  {
    m_ViewDirection = m_DefaultViewDirection;
  }

  const char* SliceNavigationController::GetViewDirectionAsString() const
  {
    const char* viewDirectionString;
    switch (m_ViewDirection)
    {
      case AnatomicalPlane::Axial:
        viewDirectionString = "Axial";
        break;

      case AnatomicalPlane::Sagittal:
        viewDirectionString = "Sagittal";
        break;

      case AnatomicalPlane::Coronal:
        viewDirectionString = "Coronal";
        break;

      case AnatomicalPlane::Original:
        viewDirectionString = "Original";
        break;

      default:
        viewDirectionString = "No view direction available";
        break;
    }
    return viewDirectionString;
  }

  void SliceNavigationController::Update()
  {
    if (!m_BlockUpdate)
    {
      if (m_ViewDirection == AnatomicalPlane::Sagittal)
      {
        this->Update(AnatomicalPlane::Sagittal, true, true, false);
      }
      else if (m_ViewDirection == AnatomicalPlane::Coronal)
      {
        this->Update(AnatomicalPlane::Coronal, false, true, false);
      }
      else if (m_ViewDirection == AnatomicalPlane::Axial)
      {
        this->Update(AnatomicalPlane::Axial, false, false, true);
      }
      else
      {
        this->Update(m_ViewDirection);
      }
    }
  }

  void SliceNavigationController::Update(AnatomicalPlane viewDirection,
                                         bool top,
                                         bool frontside,
                                         bool rotated)
  {
    if (m_BlockUpdate)
    {
      return;
    }

    if (m_InputWorldTimeGeometry.IsNull())
    {
      return;
    }

    if (0 == m_InputWorldTimeGeometry->CountTimeSteps())
    {
      return;
    }

    m_BlockUpdate = true;

    if (m_LastUpdateTime < m_InputWorldTimeGeometry->GetMTime())
    {
      Modified();
    }

    this->SetViewDirection(viewDirection);

    if (m_LastUpdateTime < GetMTime())
    {
      m_LastUpdateTime = GetMTime();

      this->CreateWorldGeometry(top, frontside, rotated);
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
        RenderingManager::GetInstance()->RequestUpdateAll();
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
        RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }

  void SliceNavigationController::SetGeometry(const itk::EventObject&)
  {
    // not implemented
  }

  void SliceNavigationController::SetGeometryTime(const itk::EventObject& geometryTimeEvent)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    const auto* timeEvent = dynamic_cast<const SliceNavigationController::GeometryTimeEvent*>(&geometryTimeEvent);
    assert(timeEvent != nullptr);

    TimeGeometry* timeGeometry = timeEvent->GetTimeGeometry();
    assert(timeGeometry != nullptr);

    auto timeStep = (int)timeEvent->GetPos();
    ScalarType timeInMS;
    timeInMS = timeGeometry->TimeStepToTimePoint(timeStep);
    timeStep = m_CreatedWorldGeometry->TimePointToTimeStep(timeInMS);
    this->GetTime()->SetPos(timeStep);
  }

  void SliceNavigationController::SetGeometrySlice(const itk::EventObject& geometrySliceEvent)
  {
    const auto* sliceEvent = dynamic_cast<const SliceNavigationController::GeometrySliceEvent*>(&geometrySliceEvent);
    assert(sliceEvent != nullptr);

    this->GetSlice()->SetPos(sliceEvent->GetPos());
  }

  void SliceNavigationController::SelectSliceByPoint(const Point3D& point)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    int selectedSlice = -1;
    try
    {
      selectedSlice = SliceNavigationHelper::SelectSliceByPoint(m_CreatedWorldGeometry, point);
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "Unable to select a slice by the given point " << point << "\n"
                 << "Reason: " << e.GetDescription();
    }

    if (-1 == selectedSlice)
    {
      return;
    }

    this->GetSlice()->SetPos(selectedSlice);

    this->SendCreatedWorldGeometryUpdate();
    // send crosshair event
    SetCrosshairEvent.Send(point);
  }

  void SliceNavigationController::ReorientSlices(const Point3D& point, const Vector3D& normal)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    PlaneOperation op(OpORIENT, point, normal);
    m_CreatedWorldGeometry->ExecuteOperation(&op);

    this->SendCreatedWorldGeometryUpdate();
  }

  void SliceNavigationController::ReorientSlices(const Point3D& point,
                                                 const Vector3D& axisVec0,
                                                 const Vector3D& axisVec1)
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return;
    }

    PlaneOperation op(OpORIENT, point, axisVec0, axisVec1);
    m_CreatedWorldGeometry->ExecuteOperation(&op);

    this->SendCreatedWorldGeometryUpdate();
  }

  const BaseGeometry* SliceNavigationController::GetCurrentGeometry3D()
  {
    if (m_CreatedWorldGeometry.IsNull())
    {
      return nullptr;
    }

    return m_CreatedWorldGeometry->GetGeometryForTimeStep(this->GetTime()->GetPos());
  }

  const PlaneGeometry* SliceNavigationController::GetCurrentPlaneGeometry()
  {
    const auto* slicedGeometry = dynamic_cast<const SlicedGeometry3D*>(this->GetCurrentGeometry3D());

    if (nullptr == slicedGeometry)
    {
      return nullptr;
    }

    return slicedGeometry->GetPlaneGeometry(this->GetSlice()->GetPos());
  }

  void SliceNavigationController::AdjustSliceStepperRange()
  {
    const auto* slicedGeometry = dynamic_cast<const SlicedGeometry3D*>(this->GetCurrentGeometry3D());

    const Vector3D& direction = slicedGeometry->GetDirectionVector();

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

  void SliceNavigationController::ExecuteOperation(Operation* operation)
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
          auto* po = dynamic_cast<PointOperation*>(operation);
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

  TimeStepType SliceNavigationController::GetSelectedTimeStep() const
  {
    return this->GetTime()->GetPos();
  }

  TimePointType SliceNavigationController::GetSelectedTimePoint() const
  {
    auto timeStep = this->GetSelectedTimeStep();

    if (m_CreatedWorldGeometry.IsNull())
    {
      return 0.0;
    }

    if (!m_CreatedWorldGeometry->IsValidTimeStep(timeStep))
    {
      mitkThrow() << "SliceNavigationController is in an invalid state. It has a time step"
                  << "selected that is not covered by its time geometry. Selected time step: " << timeStep
                  << "; TimeGeometry steps count: " << m_CreatedWorldGeometry->CountTimeSteps();
    }

    return m_CreatedWorldGeometry->TimeStepToTimePoint(timeStep);
  }

  void SliceNavigationController::CreateWorldGeometry(bool top, bool frontside, bool rotated)
  {
    // initialize the viewplane
    SlicedGeometry3D::Pointer slicedWorldGeometry;
    BaseGeometry::ConstPointer currentGeometry;

    // get the BaseGeometry (ArbitraryTimeGeometry or ProportionalTimeGeometry) of the current time step
    auto currentTimeStep = this->GetTime()->GetPos();
    if (m_InputWorldTimeGeometry->IsValidTimeStep(currentTimeStep))
    {
      currentGeometry = m_InputWorldTimeGeometry->GetGeometryForTimeStep(currentTimeStep);
    }
    else
    {
      currentGeometry = m_InputWorldTimeGeometry->GetGeometryForTimeStep(0);
    }

    if (AnatomicalPlane::Original == m_ViewDirection)
    {
      slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(
        m_InputWorldTimeGeometry->GetGeometryForTimeStep(currentTimeStep).GetPointer());
      if (slicedWorldGeometry.IsNull())
      {
        slicedWorldGeometry = SlicedGeometry3D::New();
        slicedWorldGeometry->InitializePlanes(currentGeometry, AnatomicalPlane::Original, top, frontside, rotated);
        slicedWorldGeometry->SetSliceNavigationController(this);
      }
    }
    else
    {
      slicedWorldGeometry = SlicedGeometry3D::New();
      slicedWorldGeometry->InitializePlanes(currentGeometry, m_ViewDirection, top, frontside, rotated);
      slicedWorldGeometry->SetSliceNavigationController(this);
    }

    // reset the stepper
    m_Slice->SetSteps(slicedWorldGeometry->GetSlices());
    m_Slice->SetPos(0);

    TimeStepType inputTimeSteps = m_InputWorldTimeGeometry->CountTimeSteps();
    const TimeBounds& timeBounds = m_InputWorldTimeGeometry->GetTimeBounds();
    m_Time->SetSteps(inputTimeSteps);
    m_Time->SetPos(0);
    m_Time->SetRange(timeBounds[0], timeBounds[1]);

    currentTimeStep = this->GetTime()->GetPos();
    assert(m_InputWorldTimeGeometry->GetGeometryForTimeStep(currentTimeStep).IsNotNull());

    // create new time geometry and initialize it according to the type of the 'm_InputWorldTimeGeometry'
    // the created world geometry will either have equidistant timesteps (ProportionalTimeGeometry)
    // or individual time bounds for each timestep (ArbitraryTimeGeometry)
    m_CreatedWorldGeometry = mitk::TimeGeometry::Pointer();
    if (nullptr != dynamic_cast<const mitk::ProportionalTimeGeometry*>(m_InputWorldTimeGeometry.GetPointer()))
    {
      const TimePointType minimumTimePoint = m_InputWorldTimeGeometry->TimeStepToTimePoint(currentTimeStep);
      const TimePointType stepDuration =
        m_InputWorldTimeGeometry->TimeStepToTimePoint(currentTimeStep + 1) - minimumTimePoint;

      auto createdTimeGeometry = ProportionalTimeGeometry::New();
      createdTimeGeometry->Initialize(slicedWorldGeometry, inputTimeSteps);
      createdTimeGeometry->SetFirstTimePoint(minimumTimePoint);
      createdTimeGeometry->SetStepDuration(stepDuration);

      m_CreatedWorldGeometry = createdTimeGeometry;
    }
    else
    {
      auto createdTimeGeometry = mitk::ArbitraryTimeGeometry::New();
      createdTimeGeometry->ReserveSpaceForGeometries(inputTimeSteps);
      const BaseGeometry::Pointer clonedGeometry = slicedWorldGeometry->Clone();

      for (TimeStepType i = 0; i < inputTimeSteps; ++i)
      {
        const auto bounds = m_InputWorldTimeGeometry->GetTimeBounds(i);
        createdTimeGeometry->AppendNewTimeStep(clonedGeometry, bounds[0], bounds[1]);
      }

      createdTimeGeometry->Update();
      m_CreatedWorldGeometry = createdTimeGeometry;
    }
  }
} // namespace mitk
