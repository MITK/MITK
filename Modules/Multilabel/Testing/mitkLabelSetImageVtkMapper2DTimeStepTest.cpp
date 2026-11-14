/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageGenerator.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageVtkMapper2D.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkTimeNavigationController.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkPlaneSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

namespace
{
  /** Leaves planes equal by value to the current one alone, so that neither the plane update
   *  time nor the plane's MTime moves on a time change. The mapper then has to notice the change
   *  of its data's time step on its own. */
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
}

class mitkLabelSetImageVtkMapper2DTimeStepTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageVtkMapper2DTimeStepTestSuite);
  MITK_TEST(TimeStepChanges_ReslicesNewTimeStep);
  MITK_TEST(PastDataEnd_BlanksAndBack_RegeneratesSlice);
  CPPUNIT_TEST_SUITE_END();

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  EqualPlaneKeepingRenderer::Pointer m_Renderer;
  /** The mapper refers to its node without owning it. */
  mitk::DataNode::Pointer m_Node;
  mitk::LabelSetImageVtkMapper2D::Pointer m_Mapper;

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

  /** A mapper for a segmentation with time steps [0, 1), [1, 2) and [2, 3). The first updates
   *  move time stamps of property lists lazily, which alone would let the next update
   *  regenerate, so it is updated twice. */
  void CreateUpdatedMapper()
  {
    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8, 3));

    m_Node = mitk::DataNode::New();
    m_Node->SetData(segmentation);
    m_Mapper = mitk::LabelSetImageVtkMapper2D::New();
    m_Mapper->SetDataNode(m_Node);
    m_Mapper->Update(m_Renderer);
    m_Mapper->Update(m_Renderer);
  }

public:
  void setUp() override
  {
    // Time steps [0, 1) to [3, 4), for the world and the global time, over the space of the
    // 16 x 16 x 8 segmentations of the tests.
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8)->GetGeometry(), 4);

    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_Renderer = EqualPlaneKeepingRenderer::New("mitkLabelSetImageVtkMapper2DTimeStepTest", m_RenderWindow);

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
    // The time navigation is a process-wide singleton. Renderers and slice
    // navigations read its selected time point, which derives from the
    // stepper position; removing the input does not reset that position.
    GetTimeNavigationController()->SetInputWorldTimeGeometry(nullptr);
    SelectGlobalTimeStep(0);
    m_Mapper = nullptr;
    m_Node = nullptr;
    m_Renderer = nullptr;
    m_RenderWindow = nullptr;
  }

  void TimeStepChanges_ReslicesNewTimeStep()
  {
    this->CreateUpdatedMapper();
    auto* localStorage = m_Mapper->GetLocalStorage(m_Renderer);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fixture: time step 0 resliced", mitk::TimeStepType(0),
                                 localStorage->m_ReslicerVector.at(0)->GetTimeStep());

    this->StepGlobalTime(1);
    m_Mapper->Update(m_Renderer);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Time step 1 resliced", mitk::TimeStepType(1),
                                 localStorage->m_ReslicerVector.at(0)->GetTimeStep());
  }

  void PastDataEnd_BlanksAndBack_RegeneratesSlice()
  {
    SelectGlobalTimeStep(2);
    this->CreateUpdatedMapper();
    auto* localStorage = m_Mapper->GetLocalStorage(m_Renderer);
    CPPUNIT_ASSERT_MESSAGE("Fixture: a slice of time step 2",
                           nullptr != localStorage->m_ReslicedImageVector.at(0) &&
                             mitk::TimeStepType(2) == localStorage->m_ReslicerVector.at(0)->GetTimeStep());

    this->StepGlobalTime(3);
    m_Mapper->Update(m_Renderer);

    CPPUNIT_ASSERT_MESSAGE("No slice past the data's end", nullptr == localStorage->m_ReslicedImageVector.at(0));
    CPPUNIT_ASSERT_MESSAGE("The layer shows the empty polydata past the data's end",
                           localStorage->m_EmptyPolyData.GetPointer() == localStorage->m_LayerMapperVector.at(0)->GetInput());

    this->StepGlobalTime(2);
    m_Mapper->Update(m_Renderer);

    CPPUNIT_ASSERT_MESSAGE("A slice again back at time step 2",
                           nullptr != localStorage->m_ReslicedImageVector.at(0) &&
                             mitk::TimeStepType(2) == localStorage->m_ReslicerVector.at(0)->GetTimeStep());
    // The plane source runs when the window renders, so its output is still empty here.
    CPPUNIT_ASSERT_MESSAGE("The layer shows the textured plane again back at time step 2",
                           localStorage->m_Plane.GetPointer() ==
                             localStorage->m_LayerMapperVector.at(0)->GetInputAlgorithm());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageVtkMapper2DTimeStep)
