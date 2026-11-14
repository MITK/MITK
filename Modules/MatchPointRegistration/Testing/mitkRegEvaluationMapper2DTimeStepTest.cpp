/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageGenerator.h>
#include <mitkMAPAlgorithmHelper.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRegEvaluationMapper2D.h>
#include <mitkRegEvaluationObject.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkTimeNavigationController.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

class mitkRegEvaluationMapper2DTimeStepTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRegEvaluationMapper2DTimeStepTestSuite);
  MITK_TEST(StaticTarget_LaterTimePoint_KeepsEvaluationSlice);
  MITK_TEST(DynamicTargetInMilliseconds_ReslicesTargetTimeStep);
  CPPUNIT_TEST_SUITE_END();

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  /** The evaluation object and the mapper refer to their nodes without owning them. */
  mitk::DataNode::Pointer m_TargetNode;
  mitk::DataNode::Pointer m_MovingNode;
  mitk::DataNode::Pointer m_EvaluationNode;
  mitk::RegEvaluationMapper2D::Pointer m_Mapper;

  static mitk::TimeNavigationController* GetTimeNavigationController()
  {
    return mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  }

  static void SelectGlobalTimeStep(unsigned int position)
  {
    GetTimeNavigationController()->GetStepper()->SetPos(position);
  }

  /** Four time steps of the given duration, for the world and the global time, over the space
   *  of the 16 x 16 x 8 images of the tests. */
  void SetUpTime(mitk::TimePointType stepDuration)
  {
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8)->GetGeometry(), 4);
    timeGeometry->SetStepDuration(stepDuration);
    timeGeometry->Update();

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

  /** A mapper evaluating an identity registration of a static moving image onto the given
   *  target. The first updates move time stamps of property lists lazily, which alone would let
   *  the next update regenerate, so it is updated twice. */
  void CreateUpdatedMapper(mitk::Image* target)
  {
    m_TargetNode = mitk::DataNode::New();
    m_TargetNode->SetData(target);
    m_MovingNode = mitk::DataNode::New();
    m_MovingNode->SetData(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8));

    auto evaluationObject = mitk::RegEvaluationObject::New();
    evaluationObject->SetRegistration(mitk::GenerateIdentityRegistration3D());
    evaluationObject->SetTargetNode(m_TargetNode);
    evaluationObject->SetMovingNode(m_MovingNode);

    m_EvaluationNode = mitk::DataNode::New();
    m_EvaluationNode->SetData(evaluationObject);
    mitk::RegEvaluationMapper2D::SetDefaultProperties(m_EvaluationNode);

    m_Mapper = mitk::RegEvaluationMapper2D::New();
    m_Mapper->SetDataNode(m_EvaluationNode);
    m_Mapper->Update(m_Renderer);
    m_Mapper->Update(m_Renderer);
  }

  bool ShowsEvaluationSlice()
  {
    auto* localStorage = m_Mapper->GetLocalStorage(m_Renderer);
    return nullptr != localStorage->m_EvaluationImage &&
           localStorage->m_Plane.GetPointer() == localStorage->m_Mapper->GetInputAlgorithm();
  }

public:
  void setUp() override
  {
    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_Renderer = mitk::VtkPropRenderer::New("mitkRegEvaluationMapper2DTimeStepTest", m_RenderWindow);
  }

  void tearDown() override
  {
    // The time navigation is a process-wide singleton. Renderers and slice
    // navigations read its selected time point, which derives from the
    // stepper position; removing the input does not reset that position.
    GetTimeNavigationController()->SetInputWorldTimeGeometry(nullptr);
    SelectGlobalTimeStep(0);
    m_Mapper = nullptr;
    m_EvaluationNode = nullptr;
    m_MovingNode = nullptr;
    m_TargetNode = nullptr;
    m_Renderer = nullptr;
    m_RenderWindow = nullptr;
  }

  void StaticTarget_LaterTimePoint_KeepsEvaluationSlice()
  {
    this->SetUpTime(1.0);
    this->CreateUpdatedMapper(mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8));
    CPPUNIT_ASSERT_MESSAGE("Fixture: an evaluation slice at time step 0", this->ShowsEvaluationSlice());

    SelectGlobalTimeStep(1);
    m_Mapper->Update(m_Renderer);

    CPPUNIT_ASSERT_MESSAGE("An evaluation slice of the static target at time point 1", this->ShowsEvaluationSlice());
  }

  void DynamicTargetInMilliseconds_ReslicesTargetTimeStep()
  {
    this->SetUpTime(1000.0);

    auto target = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(16, 16, 8, 4);
    auto* targetTimeGeometry = dynamic_cast<mitk::ProportionalTimeGeometry*>(target->GetTimeGeometry());
    CPPUNIT_ASSERT_MESSAGE("Fixture: a proportional target time geometry", nullptr != targetTimeGeometry);
    targetTimeGeometry->SetStepDuration(1000.0);
    targetTimeGeometry->Update();

    this->CreateUpdatedMapper(target);
    CPPUNIT_ASSERT_MESSAGE("Fixture: an evaluation slice at time step 0", this->ShowsEvaluationSlice());

    SelectGlobalTimeStep(1);
    m_Mapper->Update(m_Renderer);

    CPPUNIT_ASSERT_MESSAGE("An evaluation slice at time point 1000", this->ShowsEvaluationSlice());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The target's time step resliced", mitk::TimeStepType(1),
                                 m_Mapper->GetLocalStorage(m_Renderer)->m_Reslicer->GetTimeStep());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRegEvaluationMapper2DTimeStep)
