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
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include "mitkSurfaceToImageFilter.h"

#include <vtkPolyData.h>
#include "mitkTestFixture.h"


class mitkSurfaceToImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceToImageFilterTestSuite);
  MITK_TEST(test3DSurfaceValidOutput);
  MITK_TEST(test3DSurfaceCorrect);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::Surface::Pointer m_Surface;

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
    m_Surface = mitk::IOUtil::LoadSurface(GetTestDataFilePath("ball.stl"));
  }

  void tearDown()
  {
  }

  void test3DSurfaceValidOutput()
  {
    mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

    //todo I don't know if this image is always needed. There is no documentation of the filter. Use git blame and ask the author.
    mitk::Image::Pointer additionalInputImage = mitk::Image::New();
    additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(), *m_Surface->GetTimeGeometry());

    //Arrange the filter
    //The docu does not really tell if this is always needed. Could we skip SetImage in any case?
    surfaceToImageFilter->MakeOutputBinaryOn();
    surfaceToImageFilter->SetInput(m_Surface);
    surfaceToImageFilter->SetImage(additionalInputImage);
    surfaceToImageFilter->Update();

    CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_AnyInputImageAndModeSetToBinary_ResultIsImageWithUCHARPixelType", surfaceToImageFilter->GetOutput()->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR);
  }

  void test3DSurfaceCorrect()
  {
    mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

    //todo I don't know if this image is always needed. There is no documentation of the filter. Use git blame and ask the author.
    mitk::Image::Pointer additionalInputImage = mitk::Image::New();
    additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(), *m_Surface->GetTimeGeometry());

    //Arrange the filter
    //The docu does not really tell if this is always needed. Could we skip SetImage in any case?
    surfaceToImageFilter->MakeOutputBinaryOn();
    surfaceToImageFilter->SetInput(m_Surface);
    surfaceToImageFilter->SetImage(additionalInputImage);
    surfaceToImageFilter->Update();

    mitk::ImagePixelReadAccessor<unsigned char,3> outputReader(surfaceToImageFilter->GetOutput());
    itk::Index<3> idx;
    bool valuesCorrect = true;
    //Values outside the ball should be 0
    idx[0] =  0; idx[1] =  0, idx[2] =  0; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 0);
    idx[0] =  0; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 0);
    idx[0] = 15; idx[1] = 15, idx[2] =  0; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 0);
    idx[0] = 15; idx[1] =  0, idx[2] = 15; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 0);
    idx[0] =  4; idx[1] =  8, idx[2] = 23; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 0);
    //Values inside the ball should be 1
    idx[0] = 15; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 1);
    idx[0] = 29; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 1);
    idx[0] =  1; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 1);
    idx[0] = 15; idx[1] = 15, idx[2] =  1; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 1);
    idx[0] = 15; idx[1] =  1, idx[2] = 15; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 1);
    idx[0] =  4; idx[1] =  9, idx[2] = 23; valuesCorrect = valuesCorrect && (outputReader.GetPixelByIndex(idx) == 1);

    CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_BallSurfaceAsInput_OutputCorrect", valuesCorrect == true);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceToImageFilter)
