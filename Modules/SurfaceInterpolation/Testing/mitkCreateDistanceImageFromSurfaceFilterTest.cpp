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

#include <mitkComputeContourSetNormalsFilter.h>
#include <mitkCreateDistanceImageFromSurfaceFilter.h>
#include <mitkIOUtil.h>
#include <mitkImageAccessByItk.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

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
  void setUp() override {}
  template <typename TPixel, unsigned int VImageDimension>
  void GetImageBase(itk::Image<TPixel, VImageDimension> *input, itk::ImageBase<3>::Pointer &result)
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
      mitk::Surface::Pointer contour = dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath(s.str()))[0].GetPointer());
      contourList.push_back(contour);
    }

    mitk::Image::Pointer segmentationImage =
      dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/LiverSegmentation.nrrd"))[0].GetPointer());

    mitk::ComputeContourSetNormalsFilter::Pointer m_NormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
    mitk::CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter =
      mitk::CreateDistanceImageFromSurfaceFilter::New();

    itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
    AccessFixedDimensionByItk_1(segmentationImage, GetImageBase, 3, itkImage);
    m_InterpolateSurfaceFilter->SetReferenceImage(itkImage.GetPointer());

    for (unsigned int j = 0; j < contourList.size(); j++)
    {
      m_NormalsFilter->SetInput(j, contourList.at(j));
      m_InterpolateSurfaceFilter->SetInput(j, m_NormalsFilter->GetOutput(j));
    }

    m_InterpolateSurfaceFilter->Update();

    mitk::Image::Pointer liverDistanceImage = m_InterpolateSurfaceFilter->GetOutput();

    CPPUNIT_ASSERT(liverDistanceImage.IsNotNull());
    mitk::Image::Pointer liverDistanceImageReference =
      dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/LiverDistanceImage.nrrd"))[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("LiverDistanceImages are not equal!",
                           mitk::Equal(*(liverDistanceImageReference), *(liverDistanceImage), 0.0001, true));
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
      mitk::Surface::Pointer contour = dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath(s.str()))[0].GetPointer());
      contourList.push_back(contour);
    }

    mitk::Image::Pointer segmentationImage =
      dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/SegmentationWithHoles.nrrd"))[0].GetPointer());

    mitk::ComputeContourSetNormalsFilter::Pointer m_NormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
    mitk::CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter =
      mitk::CreateDistanceImageFromSurfaceFilter::New();

    m_NormalsFilter->SetSegmentationBinaryImage(segmentationImage);
    itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
    AccessFixedDimensionByItk_1(segmentationImage, GetImageBase, 3, itkImage);
    m_InterpolateSurfaceFilter->SetReferenceImage(itkImage.GetPointer());

    for (unsigned int j = 0; j < contourList.size(); j++)
    {
      m_NormalsFilter->SetInput(j, contourList.at(j));
      m_InterpolateSurfaceFilter->SetInput(j, m_NormalsFilter->GetOutput(j));
    }

    m_InterpolateSurfaceFilter->Update();

    mitk::Image::Pointer holeDistanceImage = m_InterpolateSurfaceFilter->GetOutput();

    CPPUNIT_ASSERT(holeDistanceImage.IsNotNull());
    mitk::Image::Pointer holesDistanceImageReference =
      dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(GetTestDataFilePath("SurfaceInterpolation/Reference/HolesDistanceImage.nrrd"))[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("HolesDistanceImages are not equal!",
                           mitk::Equal(*(holesDistanceImageReference), *(holeDistanceImage), 0.0001, true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCreateDistanceImageFromSurfaceFilter)
