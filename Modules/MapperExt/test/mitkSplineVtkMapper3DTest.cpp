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
#include <mitkRenderingTestHelper.h>
#include <mitkSplineVtkMapper3D.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkDebugLeaks.h>

class mitkSplineVtkMapper3DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSplineVtkMapper3DTestSuite);

  /// \todo Fix VTK memory leaks. Bug 19577.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(RenderSpline);
  MITK_TEST(RenderBlueSpline);
  MITK_TEST(RenderThickSpline);
  //  MITK_TEST(RenderThickTranslucentSpline);
  MITK_TEST(RenderLowResolutionSpline);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::RenderingTestHelper m_RenderingTestHelper;
  std::vector<std::string> m_CommandlineArgs;
  std::string m_PathToPointSet;

public:
  /**
   * @brief mitkSplineVtkMapper3DTestSuite Because the RenderingTestHelper does not have an
   * empty default constructor, we need this constructor to initialize the helper with a
   * resolution.
   */
  mitkSplineVtkMapper3DTestSuite() : m_RenderingTestHelper(640, 480) {}
  /**
   * @brief Setup Initialize a fresh rendering test helper and a vector of strings
   * to simulate commandline arguments for vtkTesting::Test.
   */
  void setUp()
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(640, 480);

    // disables anti-aliasing which is enabled on several graphics cards and
    // causes problems when doing a pixel-wise comparison to a reference image
    m_RenderingTestHelper.GetVtkRenderWindow()->SetMultiSamples(0);

    m_PathToPointSet =
      GetTestDataFilePath("InteractionTestData/ReferenceData/PointSetDataInteractor_PointsAdd2d3d.mps");

    // Build a command line for the vtkTesting::Test method.
    // See VTK documentation and RenderingTestHelper for more information.
    // Use the following command line option to save the difference image
    // and the test image in some tmp folder
    // m_CommandlineArgs.push_back("-T");
    // m_CommandlineArgs.push_back("/path/to/save/tmp/difference/images/");
    m_CommandlineArgs.push_back("-V");
  }

  void tearDown() {}
  void RenderSpline()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToPointSet)[0]);

    mitk::SplineVtkMapper3D::Pointer mapper = mitk::SplineVtkMapper3D::New();
    node->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

    m_RenderingTestHelper.AddNodeToStorage(node);
    m_RenderingTestHelper.SetMapperIDToRender3D();

    // reference screenshot for this test
    m_CommandlineArgs.push_back(GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/spline_linewidth_1.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderBlueSpline()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToPointSet)[0]);
    node->SetProperty("color", mitk::ColorProperty::New(0.0f, 0.0f, 1.0f));

    mitk::SplineVtkMapper3D::Pointer mapper = mitk::SplineVtkMapper3D::New();
    node->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

    m_RenderingTestHelper.AddNodeToStorage(node);
    m_RenderingTestHelper.SetMapperIDToRender3D();
    // reference screenshot for this test
    m_CommandlineArgs.push_back(
      GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/spline_blue_linewidth_1.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderThickSpline()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToPointSet)[0]);

    node->SetProperty("line width", mitk::IntProperty::New(8));

    mitk::SplineVtkMapper3D::Pointer mapper = mitk::SplineVtkMapper3D::New();
    node->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

    m_RenderingTestHelper.AddNodeToStorage(node);
    m_RenderingTestHelper.SetMapperIDToRender3D();
    // reference screenshot for this test
    m_CommandlineArgs.push_back(GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/spline_linewidth_8.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  // Currently, rending of objects with opacity < 1.0 fails on the dashboard
  // see bug 19575
  // void RenderThickTranslucentSpline()
  //{
  //  mitk::DataNode::Pointer node = mitk::DataNode::New();
  //  node->SetData(mitk::IOUtil::Load(m_PathToPointSet)[0]);
  //  node->SetOpacity(0.5f);

  //  node->SetProperty("line width", mitk::IntProperty::New(8));

  //  mitk::SplineVtkMapper3D::Pointer mapper = mitk::SplineVtkMapper3D::New();
  //  node->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

  //  m_RenderingTestHelper.AddNodeToStorage(node);
  //  m_RenderingTestHelper.SetMapperIDToRender3D();
  //  //reference screenshot for this test
  //  m_CommandlineArgs.push_back(GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/spline_translucent_linewidth_8.png"));
  //  //Convert vector of strings to argc/argv
  //  mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
  //  CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  //}

  void RenderLowResolutionSpline()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToPointSet)[0]);

    mitk::SplineVtkMapper3D::Pointer mapper = mitk::SplineVtkMapper3D::New();
    node->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

    mapper->SetSplineResolution(50);

    m_RenderingTestHelper.AddNodeToStorage(node);
    m_RenderingTestHelper.SetMapperIDToRender3D();
    // reference screenshot for this test
    m_CommandlineArgs.push_back(
      GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/spline_lowRes_linewidth_1.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSplineVtkMapper3D)
