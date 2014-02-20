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


#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>

#include "mitkSurfaceToImageFilter.h"

#include <vtkPolyData.h>
#include "mitkTestFixture.h"


class mitkSurfaceToImageFilterTest : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSTLFileReaderTestSuite);
  MITK_TEST(testReadFile);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  std::string m_SurfacePath;

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
    m_SurfacePath = GetTestDataFilePath("ball.stl");
  }

  void tearDown()
  {
  }

  void test3DSurface()
  {
    mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

     //todo I don't know if this image is always needed. There is no documentation of the filter. Use git blame and ask the author.
     mitk::Image::Pointer additionalInputImage = mitk::Image::New();
     additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(), *m_SurfacePath->GetGeometry());

     //Arrange the filter
     //The docu does not really tell if this is always needed. Could we skip SetImage in any case?
     surfaceToImageFilter->MakeOutputBinaryOn();
     surfaceToImageFilter->SetInput(m_SurfacePath);
     surfaceToImageFilter->SetImage(additionalInputImage);
     surfaceToImageFilter->Update();

     CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_AnyInputImageAndModeSetToBinary_ResultIsImageWithUCHARPixelType", surfaceToImageFilter->GetOutput()->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR);


//    CPPUNIT_ASSERT_MESSAGE("Number of Points in VtkPolyData",(n == m));

  }

  void test4DSurface()
  {

  }

};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceToImageFilter)


/*
  MITK_TEST_BEGIN( "mitkSurfaceToImageFilterTest");

  MITK_TEST_END();
  */


