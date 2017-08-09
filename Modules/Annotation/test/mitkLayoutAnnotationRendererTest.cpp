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

#include <mitkLayoutAnnotationRenderer.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkTextAnnotation2D.h>

void createTextAnnotationWithLayouter(std::vector<mitk::TextAnnotation2D::Pointer> &Annotation,
                                   mitk::LayoutAnnotationRenderer::Alignment align,
                                   mitk::BaseRenderer *renderer,
                                   int fontsize,
                                   float red,
                                   float green,
                                   float blue,
                                   int prio,
                                   std::string text)
{
  // Create a textAnnotation2D
  mitk::TextAnnotation2D::Pointer textAnnotation = mitk::TextAnnotation2D::New();

  textAnnotation->SetText(text);
  textAnnotation->SetFontSize(fontsize);
  textAnnotation->SetColor(red, green, blue);
  textAnnotation->SetOpacity(1);

  mitk::LayoutAnnotationRenderer::AddAnnotation(textAnnotation, renderer, align, 5, 5, prio);
  Annotation.push_back(textAnnotation);
}

class mitkLayoutAnnotationRendererTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLayoutAnnotationRendererTestSuite);
  MITK_TEST(Render2DAnnotation);

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
   * @brief mitkManualPlacementAnnotationRendererTestSuite Because the RenderingTestHelper does not have an
   * empty default constructor, we need this constructor to initialize the helper with a
   * resolution.
   */
  mitkLayoutAnnotationRendererTestSuite() : m_RenderingTestHelper(500, 500) {}
  /**
   * @brief Setup Initialize a fresh rendering test helper and a vector of strings
   * to simulate commandline arguments for vtkTesting::Test.
   */
  void setUp()
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(500, 500);

    m_PathToBall = GetTestDataFilePath("ball.stl");
    m_PathToImage = GetTestDataFilePath("Pic3D.nrrd");
    m_ReferenceImagePath = "RenderingTestData/ReferenceScreenshots/Annotation/";

    // Build a command line for the vtkTesting::Test method.
    // See VTK documentation and RenderingTestHelper for more information.
    // Use the following command line option to save the difference image
    // and the test image in some tmp folder
    // m_CommandlineArgs.push_back("-T");
    // m_CommandlineArgs.push_back("/path/to/save/tmp/difference/images/");
    m_CommandlineArgs.push_back("-V");
  }

  void tearDown() {}
  void Render2DAnnotation()
  {
    mitk::DataNode::Pointer ballnode = mitk::DataNode::New();
    ballnode->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    m_RenderingTestHelper.AddNodeToStorage(ballnode);

    mitk::DataNode::Pointer imagenode = mitk::DataNode::New();
    imagenode->SetData(mitk::IOUtil::Load(m_PathToImage)[0]);
    m_RenderingTestHelper.AddNodeToStorage(imagenode);

    std::string refImagePath = GetTestDataFilePath(m_ReferenceImagePath + "LayoutAnnotationRenderer.png");
    // reference screenshot for this test
    m_CommandlineArgs.push_back(refImagePath);
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    m_RenderingTestHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

    std::vector<mitk::TextAnnotation2D::Pointer> Annotation;
    mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());

    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::TopLeft, renderer, 20, 1.0, 1.0, 1.0, 1, "TopLeft1");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::TopLeft, renderer, 15, 1.0, 1.0, 1.0, 3, "TopLeft3");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::TopLeft, renderer, 25, 1.0, 0.0, 1.0, 2, "TopLeft2");

    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Top, renderer, 15, 1.0, 1.0, 1.0, 3, "Top3");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Top, renderer, 20, 1.0, 1.0, 1.0, 1, "Top1");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Top, renderer, 25, 1.0, 0.0, 1.0, 2, "Top2");

    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::TopRight, renderer, 20, 1.0, 1.0, 1.0, 1, "TopRight1");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::TopRight, renderer, 15, 1.0, 1.0, 1.0, 3, "TopRight3");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::TopRight, renderer, 25, 1.0, 0.0, 1.0, 2, "TopRight2");

    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Left, renderer, 20, 1.0, 1.0, 1.0, 1, "Left1");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Left, renderer, 15, 1.0, 1.0, 1.0, 3, "Left3");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Left, renderer, 25, 1.0, 0.0, 1.0, 2, "Left2");

    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Right, renderer, 25, 1.0, 0.0, 1.0, 2, "Right2");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Right, renderer, 20, 1.0, 1.0, 1.0, 1, "Right1");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Right, renderer, 15, 1.0, 1.0, 1.0, 3, "Right3");

    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::BottomLeft, renderer, 25, 1.0, 0.0, 1.0, 2, "BottomLeft2");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::BottomLeft, renderer, 20, 1.0, 1.0, 1.0, 1, "BottomLeft1");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::BottomLeft, renderer, 15, 1.0, 1.0, 1.0, 3, "BottomLeft3");

    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Bottom, renderer, 15, 1.0, 1.0, 1.0, 3, "Bottom3");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Bottom, renderer, 20, 1.0, 1.0, 1.0, 1, "Bottom1");
    createTextAnnotationWithLayouter(Annotation, mitk::LayoutAnnotationRenderer::Bottom, renderer, 25, 1.0, 0.0, 1.0, 2, "Bottom2");

    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::BottomRight, renderer, 25, 1.0, 0.0, 1.0, 2, "BottomRight2");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::BottomRight, renderer, 20, 1.0, 1.0, 1.0, 1, "BottomRight1");
    createTextAnnotationWithLayouter(
      Annotation, mitk::LayoutAnnotationRenderer::BottomRight, renderer, 15, 1.0, 1.0, 1.0, 3, "BottomRight3");

    m_RenderingTestHelper.Render();
//    m_RenderingTestHelper.SaveReferenceScreenShot(refImagePath);
    m_RenderingTestHelper.SetAutomaticallyCloseRenderWindow(true);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkLayoutAnnotationRenderer)
