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
#include <mitkImageRegionAccessor.h>

#include "mitkSurfaceToImageFilter.h"

#include <vtkPolyData.h>
#include "mitkTestFixture.h"


class mitkSurfaceToImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceToImageFilterTestSuite);
  MITK_TEST(test3DSurfaceValidOutput);
  MITK_TEST(test3DSurfaceCorrect);
  MITK_TEST(test3DSurfaceIn4DImage);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::Surface::Pointer m_Surface;

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override
  {
    m_Surface = mitk::IOUtil::LoadSurface(GetTestDataFilePath("ball.stl"));
  }

  void tearDown() override
  {
  }

  void test3DSurfaceValidOutput()
  {
    mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

    mitk::Image::Pointer additionalInputImage = mitk::Image::New();
    additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(), *m_Surface->GetTimeGeometry());

    //Arrange the filter
    surfaceToImageFilter->MakeOutputBinaryOn();
    surfaceToImageFilter->SetInput(m_Surface);
    surfaceToImageFilter->SetImage(additionalInputImage);
    surfaceToImageFilter->Update();

    CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_AnyInputImageAndModeSetToBinary_ResultIsImageWithUCHARPixelType", surfaceToImageFilter->GetOutput()->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR);

    surfaceToImageFilter->SetUShortBinaryPixelType(true);
    surfaceToImageFilter->Update();

    CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_AnyInputImageAndModeSetToBinary_ResultIsImageWithUCHARPixelType", surfaceToImageFilter->GetOutput()->GetPixelType().GetComponentType() == itk::ImageIOBase::USHORT);
  }

  void test3DSurfaceCorrect()
  {
    mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

    //todo I don't know if this image is always needed. There is no documentation of the filter. Use git blame and ask the author.
    mitk::Image::Pointer additionalInputImage = mitk::Image::New();
    unsigned int* dims = new unsigned int[3];
    dims[0] = 32;
    dims[1] = 32;
    dims[2] = 32;
    additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(),3,dims);
    additionalInputImage->SetOrigin(m_Surface->GetGeometry()->GetOrigin());
    additionalInputImage->GetGeometry()->SetIndexToWorldTransform(m_Surface->GetGeometry()->GetIndexToWorldTransform());
    //Arrange the filter
    //The docu does not really tell if this is always needed. Could we skip SetImage in any case?
    surfaceToImageFilter->MakeOutputBinaryOn();
    surfaceToImageFilter->SetInput(m_Surface);
    surfaceToImageFilter->SetImage(additionalInputImage);
    surfaceToImageFilter->Update();

    mitk::ImageRegionAccessor outputReader(surfaceToImageFilter->GetOutput());
    itk::Index<3> idx;
    bool valuesCorrect = true;
    //Values outside the ball should be 0
    idx[0] =  0; idx[1] =  0, idx[2] =  0; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] =  0; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] = 15; idx[1] = 15, idx[2] =  0; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] = 15; idx[1] =  0, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] =  5; idx[1] =  9, idx[2] = 23; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    //Values inside the ball should be 1
    idx[0] = 15; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] = 31; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] =  2; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] = 15; idx[1] = 15, idx[2] =  2; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] = 15; idx[1] =  2, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] =  6; idx[1] =  9, idx[2] = 23; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);

    CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_BallSurfaceAsInput_OutputCorrect", valuesCorrect == true);
  }

  void test3DSurfaceIn4DImage()
  {
    mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();

    mitk::Image::Pointer additionalInputImage = mitk::Image::New();
    unsigned int* dims = new unsigned int[4];
    dims[0] = 32;
    dims[1] = 32;
    dims[2] = 32;
    dims[3] = 2;
    additionalInputImage->Initialize( mitk::MakeScalarPixelType<unsigned int>(),4,dims);
    additionalInputImage->SetOrigin(m_Surface->GetGeometry()->GetOrigin());
    additionalInputImage->GetGeometry()->SetIndexToWorldTransform(m_Surface->GetGeometry()->GetIndexToWorldTransform());
    mitk::Image::Pointer secondStep = additionalInputImage->Clone();
    unsigned int size = sizeof(unsigned char);
    for (unsigned int i = 0; i < secondStep->GetDimension(); ++i)
      size *= secondStep->GetDimension(i);
    memset( secondStep->GetVolumeData()->GetData(), 1, size );
    additionalInputImage->GetTimeGeometry()->Expand(2);
    additionalInputImage->GetGeometry(1)->SetSpacing(secondStep->GetGeometry()->GetSpacing());
    additionalInputImage->GetGeometry(1)->SetOrigin(secondStep->GetGeometry()->GetOrigin());
    additionalInputImage->GetGeometry(1)->SetIndexToWorldTransform(secondStep->GetGeometry()->GetIndexToWorldTransform());
    additionalInputImage->SetImportVolume(secondStep->GetVolumeData(),0);
    additionalInputImage->SetImportVolume(secondStep->GetVolumeData(),1);

    //Arrange the filter
    surfaceToImageFilter->MakeOutputBinaryOn();
    surfaceToImageFilter->SetInput(m_Surface);
    surfaceToImageFilter->SetImage(additionalInputImage);

    surfaceToImageFilter->Update();

    mitk::ImageRegionAccessor outputReader(surfaceToImageFilter->GetOutput());
    itk::Index<3> idx;
    bool valuesCorrect = true;
    //Values outside the ball should be 0
    idx[0] =  0; idx[1] =  0, idx[2] =  0; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] =  0; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] = 15; idx[1] = 15, idx[2] =  0; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] = 15; idx[1] =  0, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    idx[0] =  5; idx[1] =  9, idx[2] = 23; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 0);
    //Values inside the ball should be 1   hould be 1    idx[0] = 15; idx[1] = 15, idx[2] = 15; idx[3] = 0; valuesCorrect = valuesCorrect && (outputReader.GetPixel(idx) == 1);
    idx[0] = 31; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] =  2; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] = 15; idx[1] = 15, idx[2] =  2; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] = 15; idx[1] =  2, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    idx[0] =  6; idx[1] =  9, idx[2] = 23; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx) == 1);
    //Values inside the ball but in the second timestep hould be 0    idx[0] = 15; idx[1] = 15, idx[2] = 15; idx[3] = 1; valuesCorrect = valuesCorrect && (outputReader.GetPixel(idx) == 0);
    idx[0] = 31; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx,1) == 0);
    idx[0] =  2; idx[1] = 15, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx,1) == 0);
    idx[0] = 15; idx[1] = 15, idx[2] =  2; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx,1) == 0);
    idx[0] = 15; idx[1] =  2, idx[2] = 15; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx,1) == 0);
    idx[0] =  6; idx[1] =  9, idx[2] = 23; valuesCorrect = valuesCorrect && (*(unsigned char*)outputReader.getPixel(idx,1) == 0);

    CPPUNIT_ASSERT_MESSAGE("SurfaceToImageFilter_BallSurfaceAsInput_Output4DCorrect", valuesCorrect == true);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceToImageFilter)
