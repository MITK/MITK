/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include <mitkIOUtil.h>
#include <mitkNodePredicateDataType.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingTestHelper.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkTimeNavigationController.h>

// VTK
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

#include <string>

class mitkSurfaceVtkMapper2DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkMapper2DTestSuite);
  MITK_TEST(RenderBall);
  MITK_TEST(RenderOpaqueBall);
  MITK_TEST(RenderRedBall);
  MITK_TEST(RenderBallWithGeometry);
  MITK_TEST(RenderRedBinary);
  MITK_TEST(RenderEmptyTimeStep_DrawsNothing);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::RenderingTestHelper m_RenderingTestHelper;
  std::vector<std::string> m_CommandlineArgs;
  std::string m_PathToBall;
  std::string m_PathToBinary;

public:
  /**
   * @brief mitkSurfaceVtkMapper2DTestSuite Because the RenderingTestHelper does not have an
   * empty default constructor, we need this constructor to initialize the helper with a
   * resolution.
   */
  mitkSurfaceVtkMapper2DTestSuite() : m_RenderingTestHelper(640, 480) {}
  /**
   * @brief Setup Initialize a fresh rendering test helper and a vector of strings
   * to simulate commandline arguments for vtkTesting::Test.
   */
  void setUp()
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(640, 480);

    m_PathToBall = GetTestDataFilePath("ball.stl");
    m_PathToBinary = GetTestDataFilePath("binary.stl");

    // Build a command line for the vtkTesting::Test method.
    // See VTK documentation and RenderingTestHelper for more information.
    // Use the following command line option to save the difference image
    // and the test image in some tmp folder
    // m_CommandlineArgs.push_back("-T");
    // m_CommandlineArgs.push_back("/path/to/save/tmp/difference/images/");
    m_CommandlineArgs.push_back("-V");
  }

  void tearDown() {}
  void RenderBall()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/ball640x480REF.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderBallWithGeometry()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    // Modify the geometry a little
    mitk::Vector3D spacing;
    spacing[0] = 0.5;
    spacing[1] = 1.5;
    spacing[2] = 0.75;
    node->GetData()->GetGeometry()->SetSpacing(spacing);
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(
      GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/ballWithGeometry640x480REF.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderOpaqueBall()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    node->SetOpacity(0.5);
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(
      GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/ballOpacity640x480REF.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderRedBall()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    node->SetProperty("color", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f));
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(
      GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/ballColorRed640x480REF.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderRedBinary()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBinary)[0]);
    node->SetProperty("color", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f));
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(
      GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/binaryColorRed640x480REF.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderEmptyTimeStep_DrawsNothing()
  {
    // A sphere in the first and the last time step and nothing in between,
    // like polygon models of segmentations at time steps without the label.
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(10.0);
    sphere->Update();

    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(sphere->GetOutput(), 0);
    surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), 1);
    surface->SetVtkPolyData(sphere->GetOutput(), 2);

    auto node = mitk::DataNode::New();
    node->SetData(surface);
    m_RenderingTestHelper.AddNodeToStorage(node);

    auto *stepper = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetStepper();

    // The empty time step is entered from both of its neighbors.
    for (const unsigned int timeStep : { 0u, 1u, 2u, 1u })
    {
      stepper->SetPos(timeStep);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Time step " + std::to_string(timeStep),
                                   timeStep != 1,
                                   m_RenderingTestHelper.RendersAnything());
    }

    // A render that changes nothing about the surface or the slice, like
    // zooming, must not bring back the cut of the previous time step.
    m_RenderingTestHelper.GetVtkRenderer()->GetActiveCamera()->Zoom(1.5);
    CPPUNIT_ASSERT_MESSAGE("Time step 1 after zooming", !m_RenderingTestHelper.RendersAnything());
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper2D)
