/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageGenerator.h>
#include <mitkImageVtkMapper2D.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSurface.h>
#include <mitkSurfaceVtkMapper2D.h>
#include <mitkTimeNavigationController.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkDataSet.h>
#include <vtkMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

namespace
{
  /** Leaves planes equal by value to the current one alone, so that neither the plane update
   *  time nor the plane's MTime moves on a time change. Mappers then have to notice the change
   *  of their data's time step on their own. */
  class EqualPlaneKeepingRenderer : public mitk::VtkPropRenderer
  {
  public:
    mitkClassMacro(EqualPlaneKeepingRenderer, mitk::VtkPropRenderer);
    mitkNewMacro2Param(Self, const char*, vtkRenderWindow*);

  protected:
    EqualPlaneKeepingRenderer(const char* name, vtkRenderWindow* renderWindow)
      : VtkPropRenderer(name, renderWindow)
    {
    }

    void SetCurrentWorldPlaneGeometry(const mitk::PlaneGeometry* geometry) override
    {
      const auto* currentGeometry = this->GetCurrentWorldPlaneGeometry();
      if (nullptr != currentGeometry && nullptr != geometry &&
          mitk::Equal(*currentGeometry, *geometry, mitk::eps, false))
      {
        return;
      }

      Superclass::SetCurrentWorldPlaneGeometry(geometry);
    }
  };

  /** Makes the local storage accessible, which ImageVtkMapper2D keeps protected. */
  class InspectableImageVtkMapper2D : public mitk::ImageVtkMapper2D
  {
  public:
    mitkClassMacro(InspectableImageVtkMapper2D, mitk::ImageVtkMapper2D);
    itkFactorylessNewMacro(Self);

    using Superclass::GetLocalStorage;
  };
}

class mitkMapperOwnTimeStepTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMapperOwnTimeStepTestSuite);
  MITK_TEST(ImageVtkMapper2D_TimeStepChanges_ReslicesNewTimeStep);
  MITK_TEST(ImageVtkMapper2D_PastDataEndAndBack_ReslicesAgain);
  MITK_TEST(SurfaceVtkMapper2D_TimeStepChanges_CutsNewTimeStep);
  MITK_TEST(SurfaceVtkMapper2D_PastDataEndAndBack_ShowsCutAgain);
  CPPUNIT_TEST_SUITE_END();

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  EqualPlaneKeepingRenderer::Pointer m_Renderer;

  static mitk::TimeNavigationController* GetTimeNavigationController()
  {
    return mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  }

  static void SelectGlobalTimeStep(unsigned int position)
  {
    GetTimeNavigationController()->GetStepper()->SetPos(position);
  }

  /** Selects the given global time step and checks that the renderer's plane stays as it was. */
  void StepGlobalTime(unsigned int position)
  {
    const auto planeUpdateTime = m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime();
    const auto* plane = m_Renderer->GetCurrentWorldPlaneGeometry();
    const auto planeMTime = plane->GetMTime();

    SelectGlobalTimeStep(position);

    CPPUNIT_ASSERT_MESSAGE("Fixture: the plane update time stays",
                           planeUpdateTime == m_Renderer->GetCurrentWorldPlaneGeometryUpdateTime());
    CPPUNIT_ASSERT_MESSAGE("Fixture: the plane stays",
                           plane == m_Renderer->GetCurrentWorldPlaneGeometry() && planeMTime == plane->GetMTime());
  }

  /** Through the base class, since SurfaceVtkMapper2D makes Update() protected. */
  void UpdateMapper(mitk::Mapper* mapper)
  {
    mapper->Update(m_Renderer);
  }

  /** The first updates move time stamps of property lists lazily, which alone would let the
   *  next update regenerate. */
  void UpdateTwice(mitk::Mapper* mapper)
  {
    this->UpdateMapper(mapper);
    this->UpdateMapper(mapper);
  }

  /** Three time steps, [0, 1), [1, 2) and [2, 3), with spheres of different resolutions. */
  static mitk::Surface::Pointer CreateSurface()
  {
    auto surface = mitk::Surface::New();
    for (mitk::TimeStepType timeStep = 0; timeStep < 3; ++timeStep)
    {
      auto sphere = vtkSmartPointer<vtkSphereSource>::New();
      sphere->SetCenter(8.0, 8.0, 4.0);
      sphere->SetRadius(10.0);
      sphere->SetThetaResolution(8 + 4 * static_cast<int>(timeStep));
      sphere->Update();
      surface->SetVtkPolyData(sphere->GetOutput(), timeStep);
    }

    const auto* timeGeometry = surface->GetTimeGeometry();
    CPPUNIT_ASSERT_MESSAGE("Fixture: time steps [0, 1), [1, 2) and [2, 3)",
                           mitk::TimeStepType(3) == timeGeometry->CountTimeSteps() &&
                             mitk::TimeStepType(2) == timeGeometry->TimePointToTimeStep(2.0) &&
                             !timeGeometry->IsValidTimePoint(3.0));
    return surface;
  }

  /** The number of points of the cut on display. The local storage is out of reach, since its
   *  constructor is not exported; the first part of the prop assembly shows the cut. */
  static vtkIdType CountCutPoints(mitk::SurfaceVtkMapper2D* mapper, mitk::BaseRenderer* renderer)
  {
    auto* assembly = vtkAssembly::SafeDownCast(mapper->GetVtkProp(renderer));
    if (nullptr == assembly)
    {
      return -1;
    }

    auto* parts = assembly->GetParts();
    parts->InitTraversal();
    auto* actor = vtkActor::SafeDownCast(parts->GetNextProp3D());
    auto* cut = nullptr != actor && nullptr != actor->GetMapper() ? actor->GetMapper()->GetInput() : nullptr;
    return nullptr != cut ? cut->GetNumberOfPoints() : -1;
  }

  /** The number of points of the cut a new mapper shows at the selected time. */
  vtkIdType CountCutPointsOfNewMapper(mitk::DataNode* node)
  {
    auto mapper = mitk::SurfaceVtkMapper2D::New();
    mapper->SetDataNode(node);
    this->UpdateMapper(mapper);
    return CountCutPoints(mapper, m_Renderer);
  }

public:
  void setUp() override
  {
    // Time steps [0, 1) to [3, 4), for the world and the global time, over the space of the
    // 16 x 16 x 8 images of the tests.
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8)->GetGeometry(), 4);

    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_Renderer = EqualPlaneKeepingRenderer::New("mitkMapperOwnTimeStepTest", m_RenderWindow);

    auto* sliceNavigationController = m_Renderer->GetSliceNavigationController();
    sliceNavigationController->SetInputWorldTimeGeometry(timeGeometry);
    sliceNavigationController->SetViewDirection(mitk::AnatomicalPlane::Axial);
    sliceNavigationController->Update();
    sliceNavigationController->GetStepper()->SetPos(4);

    auto* timeNavigationController = GetTimeNavigationController();
    timeNavigationController->SetInputWorldTimeGeometry(timeGeometry);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the time navigation takes the geometry",
                           timeGeometry.GetPointer() == timeNavigationController->GetInputWorldTimeGeometry());
    timeNavigationController->Update();
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

  void ImageVtkMapper2D_TimeStepChanges_ReslicesNewTimeStep()
  {
    auto node = mitk::DataNode::New();
    node->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8, 3));
    auto mapper = InspectableImageVtkMapper2D::New();
    mapper->SetDataNode(node);
    this->UpdateTwice(mapper);

    auto* localStorage = mapper->GetLocalStorage(m_Renderer);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: time step 0 resliced", mitk::TimeStepType(0),
                                 localStorage->m_Reslicer->GetTimeStep());

    this->StepGlobalTime(1);
    this->UpdateMapper(mapper);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time step 1 resliced", mitk::TimeStepType(1), localStorage->m_Reslicer->GetTimeStep());
  }

  void ImageVtkMapper2D_PastDataEndAndBack_ReslicesAgain()
  {
    SelectGlobalTimeStep(2);

    auto node = mitk::DataNode::New();
    node->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8, 3));
    auto mapper = InspectableImageVtkMapper2D::New();
    mapper->SetDataNode(node);
    this->UpdateTwice(mapper);

    auto* localStorage = mapper->GetLocalStorage(m_Renderer);
    CPPUNIT_ASSERT_MESSAGE("Fixture: a slice of time step 2",
                           nullptr != localStorage->m_ReslicedImage &&
                             mitk::TimeStepType(2) == localStorage->m_Reslicer->GetTimeStep());

    this->StepGlobalTime(3);
    this->UpdateMapper(mapper);
    CPPUNIT_ASSERT_MESSAGE("Fixture: no slice past the data's end", nullptr == localStorage->m_ReslicedImage);

    this->StepGlobalTime(2);
    this->UpdateMapper(mapper);
    CPPUNIT_ASSERT_MESSAGE("A slice again back at time step 2", nullptr != localStorage->m_ReslicedImage);
  }

  void SurfaceVtkMapper2D_TimeStepChanges_CutsNewTimeStep()
  {
    auto node = mitk::DataNode::New();
    node->SetData(CreateSurface());
    auto mapper = mitk::SurfaceVtkMapper2D::New();
    mapper->SetDataNode(node);
    this->UpdateTwice(mapper);
    const auto cutPointsOfTimeStep0 = CountCutPoints(mapper, m_Renderer);

    this->StepGlobalTime(1);
    const auto cutPointsOfTimeStep1 = this->CountCutPointsOfNewMapper(node);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the cuts of time steps 0 and 1 differ",
                           0 < cutPointsOfTimeStep0 && 0 < cutPointsOfTimeStep1 &&
                             cutPointsOfTimeStep0 != cutPointsOfTimeStep1);

    this->UpdateMapper(mapper);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time step 1 cut", cutPointsOfTimeStep1, CountCutPoints(mapper, m_Renderer));
  }

  void SurfaceVtkMapper2D_PastDataEndAndBack_ShowsCutAgain()
  {
    SelectGlobalTimeStep(2);

    auto node = mitk::DataNode::New();
    node->SetData(CreateSurface());
    auto mapper = mitk::SurfaceVtkMapper2D::New();
    mapper->SetDataNode(node);
    this->UpdateTwice(mapper);

    auto* prop = mapper->GetVtkProp(m_Renderer);
    CPPUNIT_ASSERT_MESSAGE("Fixture: the cut of time step 2 shown", prop->GetVisibility());

    this->StepGlobalTime(3);
    this->UpdateMapper(mapper);
    CPPUNIT_ASSERT_MESSAGE("Fixture: hidden past the data's end", !prop->GetVisibility());

    this->StepGlobalTime(2);
    this->UpdateMapper(mapper);
    CPPUNIT_ASSERT_MESSAGE("Shown again back at time step 2", prop->GetVisibility());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMapperOwnTimeStep)
