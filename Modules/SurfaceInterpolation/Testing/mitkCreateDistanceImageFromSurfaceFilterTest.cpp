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

#include <mitkImageAccessByItk.h>
#include <mitkCreateDistanceImageFromSurfaceFilter.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkComputeContourSetNormalsFilter.h>

#include <vtkDebugLeaks.h>

class mitkCreateDistanceImageFromSurfaceFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCreateDistanceImageFromSurfaceFilterTestSuite);
  vtkDebugLeaks::SetExitError(0);
  MITK_TEST(TestCreateDistanceImageForLiver);
  MITK_TEST(TestCreateDistanceImageForTube);
  CPPUNIT_TEST_SUITE_END();

private:

  std::vector<mitk::Surface::Pointer> contourList;

public:

  void setUp()
  {

  }

  template<typename TPixel, unsigned int VImageDimension>
  void GetImageBase(itk::Image<TPixel, VImageDimension>* input, itk::ImageBase<3>::Pointer& result)
  {
    result->Graft(input);
  }

  // Interpolate the shape of a liver
  void TestCreateDistanceImageForLiver()
  {
    // That's the number of available liver contours in MITK-Data
    unsigned int NUMBER_OF_LIVER_CONTOURS = 18;

    for (unsigned int i = 0; i <= NUMBER_OF_LIVER_CONTOURS; ++i)
    {
      std::stringstream s;
      s << "SurfaceInterpolation/InterpolateLiver/LiverContourWithNormals_";
      s << i;
      s << ".vtk";
      mitk::Surface::Pointer contour = mitk::IOUtil::LoadSurface(GetTestDataFilePath(s.str()));
      contourList.push_back(contour);
    }

    mitk::Image::Pointer segmentationImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("SurfaceInterpolation/Reference/LiverSegmentation.nrrd"));

    mitk::ComputeContourSetNormalsFilter::Pointer m_NormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
    mitk::CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter = mitk::CreateDistanceImageFromSurfaceFilter::New();

    itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
    AccessFixedDimensionByItk_1( segmentationImage, GetImageBase, 3, itkImage );
    m_InterpolateSurfaceFilter->SetReferenceImage( itkImage.GetPointer() );

    for (unsigned int j = 0; j < contourList.size(); j++)
    {
      m_NormalsFilter->SetInput(j, contourList.at(j));
      m_InterpolateSurfaceFilter->SetInput(j, m_NormalsFilter->GetOutput(j));
    }

    m_InterpolateSurfaceFilter->Update();

    mitk::Image::Pointer liverDistanceImage = m_InterpolateSurfaceFilter->GetOutput();

    CPPUNIT_ASSERT(liverDistanceImage.IsNotNull());
    mitk::Image::Pointer liverDistanceImageReference = mitk::IOUtil::LoadImage(GetTestDataFilePath("SurfaceInterpolation/Reference/LiverDistanceImage.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("LiverDistanceImages are not equal!", mitk::Equal(*(liverDistanceImageReference), *(liverDistanceImage), 0.0001, true));
  }



  void TestCreateDistanceImageForTube()
  {
    // That's the number of available contours with holes in MITK-Data
    unsigned int NUMBER_OF_TUBE_CONTOURS = 5;

    for (unsigned int i = 0; i < NUMBER_OF_TUBE_CONTOURS; ++i)
    {
      std::stringstream s;
      s << "SurfaceInterpolation/InterpolateWithHoles/ContourWithHoles_";
      s << i;
      s << ".vtk";
      mitk::Surface::Pointer contour = mitk::IOUtil::LoadSurface(GetTestDataFilePath(s.str()));
      contourList.push_back(contour);
    }

    mitk::Image::Pointer segmentationImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("SurfaceInterpolation/Reference/SegmentationWithHoles.nrrd"));

    mitk::ComputeContourSetNormalsFilter::Pointer m_NormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
    mitk::CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter = mitk::CreateDistanceImageFromSurfaceFilter::New();

    m_NormalsFilter->SetSegmentationBinaryImage(segmentationImage);
    itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
    AccessFixedDimensionByItk_1( segmentationImage, GetImageBase, 3, itkImage );
    m_InterpolateSurfaceFilter->SetReferenceImage( itkImage.GetPointer() );

    for (unsigned int j = 0; j < contourList.size(); j++)
    {
      m_NormalsFilter->SetInput(j, contourList.at(j));
      m_InterpolateSurfaceFilter->SetInput(j, m_NormalsFilter->GetOutput(j));
    }

    m_InterpolateSurfaceFilter->Update();

    mitk::Image::Pointer holeDistanceImage = m_InterpolateSurfaceFilter->GetOutput();

    CPPUNIT_ASSERT(holeDistanceImage.IsNotNull());
    mitk::Image::Pointer holesDistanceImageReference = mitk::IOUtil::LoadImage(GetTestDataFilePath("SurfaceInterpolation/Reference/HolesDistanceImage.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("HolesDistanceImages are not equal!", mitk::Equal(*(holesDistanceImageReference), *(holeDistanceImage), 0.0001, true));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkCreateDistanceImageFromSurfaceFilter)
