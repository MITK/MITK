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

#include <mitkManualPlacementAnnotationRenderer.h>
#include <mitkLabelAnnotation3D.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkLabelAnnotation3DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelAnnotation3DTestSuite);
  MITK_TEST(Render2DAnnotation);
  MITK_TEST(Render3DAnnotation);

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
  mitkLabelAnnotation3DTestSuite() : m_RenderingTestHelper(300, 300) {}
  /**
   * @brief Setup Initialize a fresh rendering test helper and a vector of strings
   * to simulate commandline arguments for vtkTesting::Test.
   */
  void setUp()
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(300, 300);

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
    std::string refImagePath = GetTestDataFilePath(m_ReferenceImagePath + "mitkLabelAnnotation2D.png");
    // reference screenshot for this test
    m_CommandlineArgs.push_back(refImagePath);
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    m_RenderingTestHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

    mitk::PointSet::Pointer pointset = mitk::PointSet::New();
    mitk::LabelAnnotation3D::Pointer label3d = mitk::LabelAnnotation3D::New();
    mitk::Point3D offset;
    offset[0] = .5;
    offset[1] = .5;
    offset[2] = .5;

    std::vector<std::string> labels;
    unsigned long idx = 0;
    for (int i = -10; i < 10; i += 4)
    {
      for (int j = -10; j < 10; j += 4)
      {
        mitk::Point3D point;
        point[0] = i;
        point[1] = j;
        point[2] = (i * j) / 10;
        pointset->InsertPoint(idx++, point);
        labels.push_back("test");
      }
    }

    label3d->SetLabelCoordinates(pointset);
    label3d->SetLabelVector(labels);
    label3d->SetOffsetVector(offset);

    mitk::DataNode::Pointer datanode = mitk::DataNode::New();
    datanode->SetData(pointset);
    datanode->SetName("pointSet");
    m_RenderingTestHelper.AddNodeToStorage(datanode);

    mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());
    mitk::ManualPlacementAnnotationRenderer::AddAnnotation(label3d.GetPointer(), renderer);

    m_RenderingTestHelper.Render();
//    m_RenderingTestHelper.SaveReferenceScreenShot(refImagePath);
    m_RenderingTestHelper.SetAutomaticallyCloseRenderWindow(true);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void Render3DAnnotation()
  {
    std::string refImagePath = GetTestDataFilePath(m_ReferenceImagePath + "mitkLabelAnnotation3D.png");
    // reference screenshot for this test
    m_CommandlineArgs.push_back(refImagePath);
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    m_RenderingTestHelper.SetMapperIDToRender3D();

    mitk::PointSet::Pointer pointset = mitk::PointSet::New();
    mitk::LabelAnnotation3D::Pointer label3d = mitk::LabelAnnotation3D::New();
    mitk::Point3D offset;
    offset[0] = .5;
    offset[1] = .5;
    offset[2] = .5;

    std::vector<std::string> labels;
    unsigned long idx = 0;
    for (int i = -10; i < 10; i += 4)
    {
      for (int j = -10; j < 10; j += 4)
      {
        mitk::Point3D point;
        point[0] = i;
        point[1] = j;
        point[2] = (i * j) / 10;
        pointset->InsertPoint(idx++, point);
        labels.push_back("test");
      }
    }

    label3d->SetLabelCoordinates(pointset);
    label3d->SetLabelVector(labels);
    label3d->SetOffsetVector(offset);

    mitk::DataNode::Pointer datanode = mitk::DataNode::New();
    datanode->SetData(pointset);
    datanode->SetName("pointSet");
    m_RenderingTestHelper.AddNodeToStorage(datanode);

    mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(m_RenderingTestHelper.GetVtkRenderWindow());
    mitk::ManualPlacementAnnotationRenderer::AddAnnotation(label3d.GetPointer(), renderer);

    m_RenderingTestHelper.Render();
//    m_RenderingTestHelper.SaveReferenceScreenShot(refImagePath);
    m_RenderingTestHelper.SetAutomaticallyCloseRenderWindow(true);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkLabelAnnotation3D)
