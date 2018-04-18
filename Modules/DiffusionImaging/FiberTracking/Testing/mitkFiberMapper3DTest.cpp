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

class mitkFiberMapper3DTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkFiberMapper3DTestSuite);
  MITK_TEST(Test1);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::Image<float, 3> ItkFloatImgType;

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

  mitk::FiberBundle::Pointer fib;
  mitk::DataNode::Pointer node;

public:

  void setUp() override
  {
    fib = mitk::IOUtil::Load<mitk::FiberBundle>(GetTestDataFilePath("DiffusionImaging/FiberFit/Cluster_0.fib"));

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

  void Test1()
  {
    mitk::RenderingTestHelper renderingHelper(640, 480);
    renderingHelper.AddNodeToStorage(node);
    renderingHelper.SetMapperIDToRender3D();
    renderingHelper.GetVtkRenderer()->SetBackground(0.0, 0.0, 0.0);
    mitk::RenderingManager::GetInstance()->InitializeViews(fib->GetGeometry(), mitk::RenderingManager::RequestType::REQUEST_UPDATE_ALL);

    renderingHelper.SaveReferenceScreenShot(mitk::IOUtil::GetTempPath()+"fib_3D.png");
    mitk::Image::Pointer test_image = mitk::IOUtil::Load<mitk::Image>(mitk::IOUtil::GetTempPath()+"fib_3D.png");
    mitk::Image::Pointer ref_image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/fib_renderingtest.png"));
    MITK_ASSERT_EQUAL(test_image, ref_image, "Check if images are equal.");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkFiberMapper3D)
