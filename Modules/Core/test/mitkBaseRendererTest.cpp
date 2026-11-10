/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkException.h>
#include <mitkGeometry3D.h>
#include <mitkPointSet.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkSliceNavigationController.h>
#include <mitkTimeNavigationController.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include <array>
#include <string>

class mitkBaseRendererTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBaseRendererTestSuite);
  MITK_TEST(SelectSliceByPoint_CurrentSlice_PlaneNotUpdated);
  MITK_TEST(SelectSliceByPoint_OtherSlice_PlaneUpdated);
  MITK_TEST(ReorientSlices_PlaneUpdated);
  MITK_TEST(SetTimeStep_EqualPlanes_PlaneUpdated);
  MITK_TEST(GetTime_OwnGridDiffersFromGlobal_IsSelectedTimePoint);
  MITK_TEST(SetPos_OwnStepUnchanged_TimePointAndPlaneUpdateTimesMove);
  MITK_TEST(SetPos_OwnGridEqualsGlobal_OwnStepIsPosition);
  MITK_TEST(GetTimeStep_TimePointOutsideOwnGrid_Clamps);
  MITK_TEST(New_GlobalTimeAtNonZeroPoint_StartsAtSelectedTimePoint);
  MITK_TEST(New_SelectedTimeStepPastGlobalGrid_ThrowsAndAcquiresNothing);
  MITK_TEST(SetTimeStep_OwnStep_ShowsItsTimePoint);
  MITK_TEST(SetTimeStep_NoWorldTimeGeometry_Throws);
  MITK_TEST(SetInteractionReferenceGeometry_GlobalTimePastReferenceGrid_DoesNotThrow);
  MITK_TEST(SetTimeStep_ReferenceVariesOverTime_AlignmentFollowsOwnTimePoint);
  CPPUNIT_TEST_SUITE_END();

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  mitk::VtkPropRenderer::Pointer m_Renderer;

  static mitk::TimeNavigationController* GetTimeNavigationController()
  {
    return mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  }

  static mitk::ProportionalTimeGeometry::Pointer CreateTimeGeometry(mitk::TimeStepType timeSteps,
                                                                    mitk::TimePointType stepDuration,
                                                                    mitk::TimePointType firstTimePoint = 0.0)
  {
    const double bounds[6] = { 0.0, 20.0, 0.0, 20.0, 0.0, 20.0 };
    auto geometry = mitk::Geometry3D::New();
    geometry->SetFloatBounds(bounds);

    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry, timeSteps);
    timeGeometry->SetFirstTimePoint(firstTimePoint);
    timeGeometry->SetStepDuration(stepDuration);
    timeGeometry->Update();
    return timeGeometry;
  }

  static mitk::BaseData::Pointer CreateData(const mitk::TimeGeometry* timeGeometry)
  {
    auto data = mitk::PointSet::New();
    data->SetClonedTimeGeometry(timeGeometry);
    return data.GetPointer();
  }

  /** The renderer's world time geometry is the one its slice navigation creates from this input. */
  void SetWorldInput(const mitk::TimeGeometry* timeGeometry)
  {
    auto* sliceNavigationController = m_Renderer->GetSliceNavigationController();
    sliceNavigationController->SetInputWorldTimeGeometry(timeGeometry);
    sliceNavigationController->Update();
  }

  static void SetGlobalTimeGeometry(const mitk::TimeGeometry* timeGeometry)
  {
    auto* timeNavigationController = GetTimeNavigationController();
    timeNavigationController->SetInputWorldTimeGeometry(timeGeometry);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the time navigation takes the geometry",
                           timeGeometry == timeNavigationController->GetInputWorldTimeGeometry());
    timeNavigationController->Update();
  }

  static void SelectGlobalTimeStep(unsigned int position)
  {
    GetTimeNavigationController()->GetStepper()->SetPos(position);
  }

  /** The time step of the world time geometry whose geometry the renderer slices. Each time step
   *  has a geometry of its own, so the current world geometry identifies it. */
  mitk::TimeStepType GetOwnTimeStep() const
  {
    const auto* worldTimeGeometry = m_Renderer->GetWorldTimeGeometry();
    for (mitk::TimeStepType timeStep = 0; timeStep < worldTimeGeometry->CountTimeSteps(); ++timeStep)
    {
      if (worldTimeGeometry->GetGeometryForTimeStep(timeStep).GetPointer() == m_Renderer->GetCurrentWorldGeometry())
      {
        return timeStep;
      }
    }
    return mitk::TIMESTEP_INVALID;
  }

public:
  void setUp() override
  {
    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_Renderer = mitk::VtkPropRenderer::New("mitkBaseRendererTest", m_RenderWindow);

    auto* sliceNavigationController = m_Renderer->GetSliceNavigationController();
    sliceNavigationController->SetInputWorldTimeGeometry(CreateTimeGeometry(2, 1.0));
    sliceNavigationController->SetViewDirection(mitk::AnatomicalPlane::Axial);
    sliceNavigationController->Update();
  }

  void tearDown() override
  {
    // The time navigation is a process-wide singleton: renderers read its time
    // point when they are constructed, and slice navigations read its time
    // step. Removing its input does not reset that step.
    GetTimeNavigationController()->SetInputWorldTimeGeometry(nullptr);
    SelectGlobalTimeStep(0);
    m_Renderer = nullptr;
    m_RenderWindow = nullptr;
  }

  void SelectSliceByPoint_CurrentSlice_PlaneNotUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    const auto origin = m_Renderer->GetCurrentWorldPlaneGeometry()->GetOrigin();
    m_Renderer->GetSliceNavigationController()->SelectSliceByPoint(origin);

    CPPUNIT_ASSERT_EQUAL(updateTime, m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
  }

  void SelectSliceByPoint_OtherSlice_PlaneUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    const auto* slicedGeometry = dynamic_cast<const mitk::SlicedGeometry3D*>(m_Renderer->GetCurrentWorldGeometry());
    CPPUNIT_ASSERT(nullptr != slicedGeometry);

    const auto otherSliceOrigin = slicedGeometry->GetPlaneGeometry(5)->GetOrigin();
    m_Renderer->GetSliceNavigationController()->SelectSliceByPoint(otherSliceOrigin);

    CPPUNIT_ASSERT(updateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
    CPPUNIT_ASSERT(mitk::Equal(otherSliceOrigin, m_Renderer->GetCurrentWorldPlaneGeometry()->GetOrigin()));
  }

  void ReorientSlices_PlaneUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();
    const auto normal = m_Renderer->GetCurrentWorldPlaneGeometry()->GetNormal();

    mitk::Vector3D tiltedNormal;
    mitk::FillVector3D(tiltedNormal, 0.0, 1.0, 1.0);

    const auto center = m_Renderer->GetCurrentWorldGeometry()->GetCenter();
    m_Renderer->GetSliceNavigationController()->ReorientSlices(center, tiltedNormal);

    CPPUNIT_ASSERT(updateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
    CPPUNIT_ASSERT(!mitk::Equal(normal, m_Renderer->GetCurrentWorldPlaneGeometry()->GetNormal()));
  }

  void SetTimeStep_EqualPlanes_PlaneUpdated()
  {
    const auto updateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    m_Renderer->SetTimeStep(1);

    CPPUNIT_ASSERT(updateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
  }

  void GetTime_OwnGridDiffersFromGlobal_IsSelectedTimePoint()
  {
    this->SetWorldInput(CreateTimeGeometry(2, 2.0));
    SetGlobalTimeGeometry(CreateTimeGeometry(3, 1.0));

    const auto fineData = CreateData(CreateTimeGeometry(3, 1.0));
    const auto coarseData = CreateData(CreateTimeGeometry(2, 2.0));

    const std::array<mitk::TimeStepType, 3> coarseTimeSteps = { 0, 0, 1 };
    for (unsigned int position = 0; position < 3; ++position)
    {
      SelectGlobalTimeStep(position);

      const auto where = " at position " + std::to_string(position);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The time point" + where, static_cast<mitk::TimePointType>(position),
                                   m_Renderer->GetTime());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The own time step" + where, coarseTimeSteps[position], this->GetOwnTimeStep());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The time step of the fine series" + where, mitk::TimeStepType(position),
                                   m_Renderer->GetTimeStep(fineData));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The time step of the coarse series" + where, coarseTimeSteps[position],
                                   m_Renderer->GetTimeStep(coarseData));
    }
  }

  void SetPos_OwnStepUnchanged_TimePointAndPlaneUpdateTimesMove()
  {
    this->SetWorldInput(CreateTimeGeometry(2, 2.0));
    SetGlobalTimeGeometry(CreateTimeGeometry(3, 1.0));
    SelectGlobalTimeStep(0);

    const auto timePointUpdateTime = m_Renderer->GetTimePointUpdateTime();
    const auto planeUpdateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();

    SelectGlobalTimeStep(1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: the own time step stays", mitk::TimeStepType(0), this->GetOwnTimeStep());
    CPPUNIT_ASSERT_MESSAGE("The time point update time moves",
                           timePointUpdateTime < m_Renderer->GetTimePointUpdateTime());
    CPPUNIT_ASSERT_MESSAGE("The plane update time moves",
                           planeUpdateTime < m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
  }

  void SetPos_OwnGridEqualsGlobal_OwnStepIsPosition()
  {
    this->SetWorldInput(CreateTimeGeometry(3, 1.0));
    SetGlobalTimeGeometry(CreateTimeGeometry(3, 1.0));

    for (unsigned int position = 0; position < 3; ++position)
    {
      SelectGlobalTimeStep(position);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("The own time step at position " + std::to_string(position),
                                   mitk::TimeStepType(position), this->GetOwnTimeStep());
    }
  }

  void GetTimeStep_TimePointOutsideOwnGrid_Clamps()
  {
    // Own grid [5, 10), [10, 15); global grid 0, 10, 20.
    this->SetWorldInput(CreateTimeGeometry(2, 5.0, 5.0));
    SetGlobalTimeGeometry(CreateTimeGeometry(3, 10.0));

    SelectGlobalTimeStep(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The own time step past the own grid's end", mitk::TimeStepType(1),
                                 this->GetOwnTimeStep());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The time point past the own grid's end", 20.0, m_Renderer->GetTime());

    SelectGlobalTimeStep(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The own time step before the own grid's start", mitk::TimeStepType(0),
                                 this->GetOwnTimeStep());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The time point before the own grid's start", 0.0, m_Renderer->GetTime());
  }

  void New_GlobalTimeAtNonZeroPoint_StartsAtSelectedTimePoint()
  {
    SetGlobalTimeGeometry(CreateTimeGeometry(3, 1.0));
    SelectGlobalTimeStep(2);

    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    auto renderer = mitk::VtkPropRenderer::New("mitkBaseRendererTest.New", renderWindow);

    CPPUNIT_ASSERT_EQUAL(2.0, renderer->GetTime());
  }

  void New_SelectedTimeStepPastGlobalGrid_ThrowsAndAcquiresNothing()
  {
    // The fixture's renderer observes the time navigation too.
    m_Renderer = nullptr;

    auto* timeNavigationController = GetTimeNavigationController();
    const mitk::TimeNavigationController::TimeEvent timeEvent(0);
    CPPUNIT_ASSERT_MESSAGE("Fixture: nothing observes the time navigation",
                           !timeNavigationController->HasObserver(timeEvent));

    // Without a geometry the time navigation sends no time event, and taking
    // a geometry without Update() keeps the position past its only time step.
    auto* stepper = timeNavigationController->GetStepper();
    stepper->SetSteps(3);
    stepper->SetPos(2);
    timeNavigationController->SetInputWorldTimeGeometry(CreateTimeGeometry(1, 1.0));
    CPPUNIT_ASSERT_THROW_MESSAGE("Fixture: the selected time point cannot be resolved",
                                 timeNavigationController->GetSelectedTimePoint(), mitk::Exception);

    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    const auto referenceCount = renderWindow->GetReferenceCount();

    CPPUNIT_ASSERT_THROW(mitk::VtkPropRenderer::New("mitkBaseRendererTest.Throwing", renderWindow), mitk::Exception);

    const bool hasTimeObserver = timeNavigationController->HasObserver(timeEvent);
    if (hasTimeObserver)
    {
      // It would dangle: later time events of this process must not reach it.
      timeNavigationController->RemoveAllObservers();
    }

    CPPUNIT_ASSERT_MESSAGE("No time observer after the failed construction", !hasTimeObserver);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The render window's reference count after the failed construction",
                                 referenceCount, renderWindow->GetReferenceCount());
  }

  void SetTimeStep_OwnStep_ShowsItsTimePoint()
  {
    this->SetWorldInput(CreateTimeGeometry(2, 2.0));

    m_Renderer->SetTimeStep(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The time point of time step 1", 2.0, m_Renderer->GetTime());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time step 1", mitk::TimeStepType(1), this->GetOwnTimeStep());

    m_Renderer->SetTimeStep(0);
    m_Renderer->SetTimeStep(5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The time point of a time step past the last one", 2.0, m_Renderer->GetTime());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A time step past the last one", mitk::TimeStepType(1), this->GetOwnTimeStep());
  }

  void SetTimeStep_NoWorldTimeGeometry_Throws()
  {
    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    auto renderer = mitk::VtkPropRenderer::New("mitkBaseRendererTest.NoWorldTimeGeometry", renderWindow);

    CPPUNIT_ASSERT_THROW(renderer->SetTimeStep(1), mitk::Exception);
  }

  void SetInteractionReferenceGeometry_GlobalTimePastReferenceGrid_DoesNotThrow()
  {
    this->SetWorldInput(CreateTimeGeometry(3, 1.0));
    SetGlobalTimeGeometry(CreateTimeGeometry(3, 1.0));
    SelectGlobalTimeStep(1);

    // Spatially equal to the world geometry; time steps [0, 1) and [1, 2).
    const auto referenceGeometry = CreateTimeGeometry(2, 1.0);
    m_Renderer->SetInteractionReferenceGeometry(referenceGeometry);
    CPPUNIT_ASSERT_MESSAGE("Fixture: aligned inside the reference's time steps",
                           m_Renderer->GetReferenceGeometryAligned());

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Stepping past the reference's time steps", SelectGlobalTimeStep(2));
    CPPUNIT_ASSERT_MESSAGE("Aligned past the reference's time steps", m_Renderer->GetReferenceGeometryAligned());

    m_Renderer->SetInteractionReferenceGeometry(nullptr);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Setting the reference past its time steps",
                                    m_Renderer->SetInteractionReferenceGeometry(referenceGeometry));
    CPPUNIT_ASSERT_MESSAGE("Aligned after setting the reference past its time steps",
                           m_Renderer->GetReferenceGeometryAligned());
  }

  void SetTimeStep_ReferenceVariesOverTime_AlignmentFollowsOwnTimePoint()
  {
    // Time step 0 is spatially equal to the world geometry, time step 1 has half its depth.
    // The alignment check compares bounding boxes, so the extent differs, not the origin.
    auto referenceGeometry = CreateTimeGeometry(2, 1.0);
    const double shallowBounds[6] = { 0.0, 20.0, 0.0, 20.0, 0.0, 10.0 };
    auto shallowGeometry = mitk::Geometry3D::New();
    shallowGeometry->SetFloatBounds(shallowBounds);
    referenceGeometry->SetTimeStepGeometry(shallowGeometry, 1);
    referenceGeometry->Update();

    m_Renderer->SetInteractionReferenceGeometry(referenceGeometry);
    CPPUNIT_ASSERT_MESSAGE("Fixture: aligned at the global time point",
                           m_Renderer->GetReferenceGeometryAligned());

    m_Renderer->SetTimeStep(1);
    CPPUNIT_ASSERT_MESSAGE("Not aligned at the renderer's own time point",
                           !m_Renderer->GetReferenceGeometryAligned());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBaseRenderer)
