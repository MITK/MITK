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

#include <mitkNodePredicateDataType.h>
#include <mitkRenderingTestHelper.h>
#include <mitkSmartPointerProperty.h>
#include <mitkTestingMacros.h>
#include <vtkSmartPointer.h>
#include <vtkRenderLargeImage.h>
#include "mitkTestingMacros.h"
#include <mitkFiberBundle.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <vtkTesting.h>
#include <thread>
#include <chrono>

class mitkFiberMapper3DTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkFiberMapper3DTestSuite);
  MITK_TEST(Default3D);
  MITK_TEST(Color3D);
  MITK_TEST(Ribbon3D);
  MITK_TEST(Tubes3D);
  MITK_TEST(Default2D);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::Image<float, 3> ItkFloatImgType;

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

  mitk::FiberBundle::Pointer fib;
  mitk::DataNode::Pointer node;

public:

  void setUp() override
  {
    fib = mitk::IOUtil::Load<mitk::FiberBundle>(GetTestDataFilePath("DiffusionImaging/Rendering/test_fibers.fib"));
    MITK_INFO << fib->GetNumFibers();
    node = mitk::DataNode::New();
    node->SetData(fib);
  }

  void tearDown() override
  {

  }

  void AddGeneratedDataToStorage(mitk::DataStorage *dataStorage)
  {
    auto node = mitk::DataNode::New();
    node->SetData(fib);

    dataStorage->Add(node);
  }

  void Default2D()
  {
    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetViewDirection(mitk::SliceNavigationController::Frontal);
    renderingHelper.SetMapperIDToRender2D();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_2D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_2D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Rendering/fib_2D.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }

  void Default3D()
  {
    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_3D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_3D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Rendering/fib_3D.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }

  void Tubes3D()
  {
    node->SetFloatProperty("shape.tuberadius", 1);

    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_tubes_3D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_tubes_3D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Rendering/fib_tubes_3D.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }

  void Ribbon3D()
  {
    node->SetFloatProperty("shape.ribbonwidth", 1);

    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_ribbon_3D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_ribbon_3D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Rendering/fib_ribbon_3D.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }

  void Line3D()
  {
    node->SetFloatProperty("shape.linewidth", 1);

    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_line_3D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_line_3D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Rendering/fib_line_3D.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }

  void Color3D()
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->SetFiberColors(255, 255, 255);

    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_color_3D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_color_3D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Rendering/fib_color_3D.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkFiberMapper3D)
