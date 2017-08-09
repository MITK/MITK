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

// MITK
#include <mitkIOUtil.h>
#include <mitkNodePredicateDataType.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkSurfaceVtkMapper2DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkMapper2DTestSuite);
  MITK_TEST(RenderBall);
  MITK_TEST(RenderOpaqueBall);
  MITK_TEST(RenderRedBall);
  MITK_TEST(RenderBallWithGeometry);
  MITK_TEST(RenderRedBinary);
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
};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper2D)
