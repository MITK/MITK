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

#include <mitkOverlayLayouter2D.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkTextOverlay2D.h>

void createTextOverlayWithLayouter(std::vector<mitk::TextOverlay2D::Pointer> &overlays,
                                   mitk::OverlayLayouter2D::Alignment align,
                                   mitk::BaseRenderer *renderer,
                                   int fontsize,
                                   float red,
                                   float green,
                                   float blue,
                                   int prio,
                                   std::string text)
{
  // Create a textOverlay2D
  mitk::TextOverlay2D::Pointer textOverlay = mitk::TextOverlay2D::New();

  textOverlay->SetText(text);
  textOverlay->SetFontSize(fontsize);
  textOverlay->SetColor(red, green, blue);
  textOverlay->SetOpacity(1);

  mitk::OverlayLayouter2D::AddOverlay(textOverlay, renderer, align, 5, 5, prio);
  overlays.push_back(textOverlay);
}

class mitkOverlayLayouter2DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOverlayLayouter2DTestSuite);
  MITK_TEST(Render2DOverlays);

  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::RenderingTestHelper m_RenderingTestHelper;
  std::vector<std::string> m_CommandlineArgs;
  std::string m_PathToBall;
  std::string m_PathToImage;
  std::string m_ReferenceImagePath;

public:
  /**
   * @brief mitkAnnotationPlacerTestSuite Because the RenderingTestHelper does not have an
   * empty default constructor, we need this constructor to initialize the helper with a
   * resolution.
   */
  mitkOverlayLayouter2DTestSuite() : m_RenderingTestHelper(500, 500) {}
  /**
   * @brief Setup Initialize a fresh rendering test helper and a vector of strings
   * to simulate commandline arguments for vtkTesting::Test.
   */
  void setUp()
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(500, 500);

    m_PathToBall = GetTestDataFilePath("ball.stl");
    m_PathToImage = GetTestDataFilePath("Pic3D.nrrd");
    m_ReferenceImagePath = "RenderingTestData/ReferenceScreenshots/Overlay/";

    // Build a command line for the vtkTesting::Test method.
    // See VTK documentation and RenderingTestHelper for more information.
    // Use the following command line option to save the difference image
    // and the test image in some tmp folder
    // m_CommandlineArgs.push_back("-T");
    // m_CommandlineArgs.push_back("/path/to/save/tmp/difference/images/");
    m_CommandlineArgs.push_back("-V");
  }

  void tearDown() {}
  void Render2DOverlays()
  {
    mitk::DataNode::Pointer ballnode = mitk::DataNode::New();
    ballnode->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    m_RenderingTestHelper.AddNodeToStorage(ballnode);

    mitk::DataNode::Pointer imagenode = mitk::DataNode::New();
    imagenode->SetData(mitk::IOUtil::Load(m_PathToImage)[0]);
    m_RenderingTestHelper.AddNodeToStorage(imagenode);

    std::string refImagePath = GetTestDataFilePath(m_ReferenceImagePath + "OverlayLayouter2D.png");
    // reference screenshot for this test
    m_CommandlineArgs.push_back(refImagePath);
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    m_RenderingTestHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

    std::vector<mitk::TextOverlay2D::Pointer> overlays;
    mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());

    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::TopLeft, renderer, 20, 1.0, 1.0, 1.0, 1, "TopLeft1");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::TopLeft, renderer, 15, 1.0, 1.0, 1.0, 3, "TopLeft3");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::TopLeft, renderer, 25, 1.0, 0.0, 1.0, 2, "TopLeft2");

    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Top, renderer, 15, 1.0, 1.0, 1.0, 3, "Top3");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Top, renderer, 20, 1.0, 1.0, 1.0, 1, "Top1");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Top, renderer, 25, 1.0, 0.0, 1.0, 2, "Top2");

    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::TopRight, renderer, 20, 1.0, 1.0, 1.0, 1, "TopRight1");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::TopRight, renderer, 15, 1.0, 1.0, 1.0, 3, "TopRight3");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::TopRight, renderer, 25, 1.0, 0.0, 1.0, 2, "TopRight2");

    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Left, renderer, 20, 1.0, 1.0, 1.0, 1, "Left1");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Left, renderer, 15, 1.0, 1.0, 1.0, 3, "Left3");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Left, renderer, 25, 1.0, 0.0, 1.0, 2, "Left2");

    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Right, renderer, 25, 1.0, 0.0, 1.0, 2, "Right2");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Right, renderer, 20, 1.0, 1.0, 1.0, 1, "Right1");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Right, renderer, 15, 1.0, 1.0, 1.0, 3, "Right3");

    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::BottomLeft, renderer, 25, 1.0, 0.0, 1.0, 2, "BottomLeft2");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::BottomLeft, renderer, 20, 1.0, 1.0, 1.0, 1, "BottomLeft1");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::BottomLeft, renderer, 15, 1.0, 1.0, 1.0, 3, "BottomLeft3");

    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Bottom, renderer, 15, 1.0, 1.0, 1.0, 3, "Bottom3");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Bottom, renderer, 20, 1.0, 1.0, 1.0, 1, "Bottom1");
    createTextOverlayWithLayouter(overlays, mitk::OverlayLayouter2D::Bottom, renderer, 25, 1.0, 0.0, 1.0, 2, "Bottom2");

    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::BottomRight, renderer, 25, 1.0, 0.0, 1.0, 2, "BottomRight2");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::BottomRight, renderer, 20, 1.0, 1.0, 1.0, 1, "BottomRight1");
    createTextOverlayWithLayouter(
      overlays, mitk::OverlayLayouter2D::BottomRight, renderer, 15, 1.0, 1.0, 1.0, 3, "BottomRight3");

    m_RenderingTestHelper.Render();
    m_RenderingTestHelper.SaveReferenceScreenShot(refImagePath);
    m_RenderingTestHelper.SetAutomaticallyCloseRenderWindow(true);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkOverlayLayouter2D)
