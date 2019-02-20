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

#include "mitkImageStatisticsCalculator.h"
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkPlanarPolygon.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <mitkStandaloneDataStorage.h>

#include <mitkIOUtil.h>
#include <mitkImageGenerator.h>
#include <mitkImagePixelWriteAccessor.h>

#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>
#include <mitkImageMaskGenerator.h>
#include <mitkImageStatisticsConstants.h>

/**
 * \brief Test class for mitkImageStatisticsCalculator
 *
 * This test covers:
 * - instantiation of an ImageStatisticsCalculator class
 * - correctness of statistics when using PlanarFigures for masking
 */
class mitkImageStatisticsCalculatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageStatisticsCalculatorTestSuite);
  MITK_TEST(TestUninitializedImage);
  MITK_TEST(TestCase1);
  MITK_TEST(TestCase2);
  MITK_TEST(TestCase3);
  MITK_TEST(TestCase4);
  MITK_TEST(TestCase5);
  MITK_TEST(TestCase6);
  MITK_TEST(TestCase7);
  MITK_TEST(TestCase8);
  MITK_TEST(TestCase9);
  MITK_TEST(TestCase10);
  MITK_TEST(TestCase11);
  MITK_TEST(TestCase12);
  MITK_TEST(TestPic3DCroppedNoMask);
  MITK_TEST(TestPic3DCroppedBinMask);
  MITK_TEST(TestPic3DCroppedMultilabelMask);
  MITK_TEST(TestPic3DCroppedPlanarFigure);
  MITK_TEST(TestUS4DCroppedNoMaskTimeStep1);
  MITK_TEST(TestUS4DCroppedBinMaskTimeStep1);
  MITK_TEST(TestUS4DCroppedMultilabelMaskTimeStep1);
  MITK_TEST(TestUS4DCroppedPlanarFigureTimeStep1);
  CPPUNIT_TEST_SUITE_END();

public:

  void TestUninitializedImage();

  void TestCase1();
  void TestCase2();
  void TestCase3();
  void TestCase4();
  void TestCase5();
  void TestCase6();
  void TestCase7();
  void TestCase8();
  void TestCase9();
  void TestCase10();
  void TestCase11();
  void TestCase12();
  
  void TestPic3DCroppedNoMask();
  void TestPic3DCroppedBinMask();
  void TestPic3DCroppedMultilabelMask();
  void TestPic3DCroppedPlanarFigure();

  void TestUS4DCroppedNoMaskTimeStep1();
  void TestUS4DCroppedBinMaskTimeStep1();
  void TestUS4DCroppedMultilabelMaskTimeStep1();
  void TestUS4DCroppedPlanarFigureTimeStep1();

private:

  mitk::Image::ConstPointer m_TestImage;

  mitk::Image::ConstPointer m_Pic3DImage;
  mitk::Image::Pointer m_Pic3DImageMask;
  mitk::Image::Pointer m_Pic3DImageMask2;
  mitk::PlanarFigure::Pointer m_Pic3DPlanarFigureAxial;
  mitk::PlanarFigure::Pointer m_Pic3DPlanarFigureSagittal;
  mitk::PlanarFigure::Pointer m_Pic3DPlanarFigureCoronal;

  mitk::Image::ConstPointer m_US4DImage;
  mitk::Image::Pointer m_US4DImageMask;
  mitk::Image::Pointer m_US4DImageMask2;
  mitk::PlanarFigure::Pointer m_US4DPlanarFigureAxial;
  mitk::PlanarFigure::Pointer m_US4DPlanarFigureSagittal;
  mitk::PlanarFigure::Pointer m_US4DPlanarFigureCoronal;

  mitk::PlaneGeometry::Pointer m_Geometry;

  // calculate statistics for the given image and planarpolygon
  const mitk::ImageStatisticsContainer::Pointer ComputeStatistics( mitk::Image::ConstPointer image,
                                                                       mitk::PlanarFigure::Pointer polygon );

  // calculate statistics for the given image and mask
  const mitk::ImageStatisticsContainer::Pointer ComputeStatistics(mitk::Image::ConstPointer image,
                                                                       mitk::Image::Pointer image_mask );

  // universal function to calculate statistics
  const mitk::ImageStatisticsContainer::Pointer ComputeStatisticsNew(mitk::Image::ConstPointer image,
                                                                                           mitk::MaskGenerator::Pointer maskGen=nullptr,
                                                                                           mitk::MaskGenerator::Pointer secondardMaskGen=nullptr,
                                                                                           unsigned short label=1);

  void VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
                        double testMean, double testSD, double testMedian=0);

  void VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
                        unsigned long N,
                        double mean,
                        double MPP,
                        double median,
                        double skewness,
                        double kurtosis,
                        double uniformity,
                        double UPP,
                        double variance,
                        double stdev,
                        double min,
                        double max,
                        double RMS,
                        double entropy,
                        vnl_vector<int> minIndex,
                        vnl_vector<int> maxIndex);
};

void mitkImageStatisticsCalculatorTestSuite::TestCase1()
{
  /*****************************
   * one whole white pixel
   * -> mean of 255 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 1:-----------------------------------------------------------------------------------";
  
  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 10.5 ; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 10.5; pnt4[1] = 4.5;
  figure1->SetControlPoint( 3, pnt4, true );
  figure1->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 255.0, 0.0, 255.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase2()
{
  /*****************************
   * half pixel in x-direction (white)
   * -> mean of 255 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 2:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 10.0 ; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 10.0; pnt4[1] = 4.5;
  figure1->SetControlPoint( 3, pnt4, true );
  figure1->GetPolyLine(0);

    auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 255.0, 0.0, 255.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase3()
{
  /*****************************
   * half pixel in diagonal-direction (white)
   * -> mean of 255 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 3:-----------------------------------------------------------------------------------";
  
  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 10.5 ; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  figure1->GetPolyLine(0);

      auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 255.0, 0.0, 255.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase4()
{
  /*****************************
   * one pixel (white) + 2 half pixels (white) + 1 half pixel (black)
   * -> mean of 191.25 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 4:-----------------------------------------------------------------------------------";
  
  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 1.1; pnt1[1] = 1.1;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 2.0; pnt2[1] = 2.0;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 3.0; pnt3[1] = 1.0;
  figure1->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 2.0; pnt4[1] = 0.0;
  figure1->SetControlPoint( 3, pnt4, true );
  figure1->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 191.25, 110.41, 242.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase5()
{
  /*****************************
   * whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 191.5 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 5:-----------------------------------------------------------------------------------";
  
  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
  figure1->SetControlPoint( 3, pnt4, true );
  figure1->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 191.50, 63.50, 134.340);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase6()
{
  /*****************************
   * quarter pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 191.5 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 6:-----------------------------------------------------------------------------------";
  
  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.25; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.25; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
  figure1->SetControlPoint( 3, pnt4, true );
  figure1->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 191.5, 63.50, 134.340);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase7()
{
  /*****************************
   * half pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 127.66 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 7:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.0; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.0; pnt3[1] = 4.0;
  figure1->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.0;
  figure1->SetControlPoint( 3, pnt4, true );
  figure1->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure1.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 127.66, 104.1, 140.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase8()
{
  /*****************************
   * whole pixel (gray)
   * -> mean of 128 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 8:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 11.5;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 11.5;
  figure2->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
  figure2->SetControlPoint( 3, pnt4, true );
  figure2->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure2.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 128.0, 0.0, 128.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase9()
{
  /*****************************
   * whole pixel (gray) + half pixel (white) in y-direction
   * -> mean of 191.5 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 9:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 12.0;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 12.0;
  figure2->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
  figure2->SetControlPoint( 3, pnt4, true );
  figure2->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure2.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 191.5, 63.50, 134.340);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase10()
{
  /*****************************
   * 2 whole pixel (white) + 2 whole pixel (black) in y-direction
   * -> mean of 127.66 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 10:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 13.5;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 13.5;
  figure2->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
  figure2->SetControlPoint( 3, pnt4, true );
  figure2->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure2.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 127.66, 104.1, 140.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase11()
{
  /*****************************
   * 9 whole pixels (white) + 3 half pixels (white)
   * + 3 whole pixel (black) [ + 3 slightly less than half pixels (black)]
   * -> mean of 204.0 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 11:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 0.5; pnt1[1] = 0.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 3.5; pnt2[1] = 3.5;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 8.4999; pnt3[1] = 3.5;
  figure2->SetControlPoint( 2, pnt3, true );
  mitk::Point2D pnt4; pnt4[0] = 5.4999; pnt4[1] = 0.5;
  figure2->SetControlPoint( 3, pnt4, true );
  figure2->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure2.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 204.0, 102.00, 242.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase12()
{
  /*****************************
   * half pixel (white) + whole pixel (white) + half pixel (black)
   * -> mean of 212.66 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 12:-----------------------------------------------------------------------------------";

  std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
  m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
  MITK_TEST_CONDITION_REQUIRED(m_TestImage.IsNotNull(), "Loaded an mitk::Image");

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED(m_Geometry.IsNotNull(), "Getting image geometry");

  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 9.5; pnt1[1] = 0.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 2.5;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 11.5; pnt3[1] = 2.5;
  figure2->SetControlPoint( 2, pnt3, true );
  figure2->GetPolyLine(0);

  auto statisticsContainer = ComputeStatistics(m_TestImage, figure2.GetPointer());
  auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

  this->VerifyStatistics(statisticsObjectTimestep0, 212.66, 59.860, 248.640);
}

void TestPic3DCroppedNoMask()
{

}
void TestPic3DCroppedBinMask()
{

}
void TestPic3DCroppedMultilabelMask()
{

}
void TestPic3DCroppedPlanarFigure()
{

}
void TestUS4DCroppedNoMaskTimeStep1()
{

}
void TestUS4DCroppedBinMaskTimeStep1()
{

}
void TestUS4DCroppedMultilabelMaskTimeStep1()
{

}
void TestUS4DCroppedPlanarFigureTimeStep1()
{

}

const mitk::ImageStatisticsContainer::Pointer mitkImageStatisticsCalculatorTestSuite::ComputeStatistics( mitk::Image::ConstPointer image, mitk::PlanarFigure::Pointer polygon )
{
  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
  statisticsCalculator->SetInputImage( image );

  statisticsCalculator->SetNBinsForHistogramStatistics(10);

  mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
  planFigMaskGen->SetInputImage(image);
  planFigMaskGen->SetPlanarFigure(polygon);

  statisticsCalculator->SetMask(planFigMaskGen.GetPointer());

  try
  {
    return statisticsCalculator->GetStatistics();
  }
  catch( ... )
  {
    return nullptr;
  }
}

const mitk::ImageStatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatistics(mitk::Image::ConstPointer image, mitk::Image::Pointer image_mask )
{
  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
  statisticsCalculator->SetInputImage(image);

  statisticsCalculator->SetNBinsForHistogramStatistics(10);

  mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
  imgMaskGen->SetImageMask(image_mask);
  statisticsCalculator->SetMask(imgMaskGen.GetPointer());

  return statisticsCalculator->GetStatistics();
}

const mitk::ImageStatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatisticsNew(mitk::Image::ConstPointer image,
                                                             mitk::MaskGenerator::Pointer maskGen,
                                                             mitk::MaskGenerator::Pointer secondardMaskGen,
                                                             unsigned short label)
{
    mitk::ImageStatisticsCalculator::Pointer imgStatCalc = mitk::ImageStatisticsCalculator::New();
    imgStatCalc->SetInputImage(image);

    if (maskGen.IsNotNull())
    {
        imgStatCalc->SetMask(maskGen.GetPointer());
        if (secondardMaskGen.IsNotNull())
        {
            imgStatCalc->SetSecondaryMask(secondardMaskGen.GetPointer());
        }
    }

    return imgStatCalc->GetStatistics(label);
}

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
                                                              double testMean, double testSD, double testMedian)
{
  auto mean = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::MEAN());
  int tmpMean = mean * 100;
  double calculatedMean = tmpMean / 100.0;
  MITK_TEST_CONDITION( calculatedMean == testMean,
                       "Calculated mean grayvalue '" << calculatedMean <<
                       "'  is equal to the desired value '" << testMean << "'" );

  auto standardDeviation = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());
  int tmpSD = standardDeviation * 100;
  double calculatedSD = tmpSD / 100.0;
  MITK_TEST_CONDITION( calculatedSD == testSD,
                       "Calculated grayvalue sd '" << calculatedSD <<
                       "'  is equal to the desired value '" << testSD <<"'" );

  auto median = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::MEDIAN());
  int tmpMedian = median * 100;
  double calculatedMedian = tmpMedian / 100.0;
  MITK_TEST_CONDITION( testMedian == calculatedMedian,
                       "Calculated median grayvalue '" << calculatedMedian <<
                       "' is equal to the desired value '" << testMedian << "'");
}

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
                                                              unsigned long N,
                                                              double mean,
                                                              double MPP,
                                                              double median,
                                                              double skewness,
                                                              double kurtosis,
                                                              double uniformity,
                                                              double UPP,
                                                              double variance,
                                                              double stdev,
                                                              double min,
                                                              double max,
                                                              double RMS,
                                                              double entropy,
                                                              vnl_vector<int> minIndex,
                                                              vnl_vector<int> maxIndex)
{
  auto numberOfVoxelsObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::VoxelCountType>(mitk::ImageStatisticsConstants::NUMBEROFVOXELS());
  auto meanObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEAN());
  auto mppObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MPP());
  auto medianObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEDIAN());
  auto skewnessObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::SKEWNESS());
  auto kurtosisObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::KURTOSIS());
  auto uniformityObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::UNIFORMITY());
  auto uppObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::UPP());
  auto varianceObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::VARIANCE());
  auto standardDeviationObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());
  auto minObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MINIMUM());
  auto maxObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MAXIMUM());
  auto rmsObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::RMS());
  auto entropyObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::ENTROPY());
  auto minIndexObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::IndexType>(mitk::ImageStatisticsConstants::MINIMUMPOSITION());
  auto maxIndexObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::IndexType>(mitk::ImageStatisticsConstants::MAXIMUMPOSITION());

    MITK_TEST_CONDITION(numberOfVoxelsObject - N == 0, "calculated N: " << numberOfVoxelsObject << " expected N: " << N);
    MITK_TEST_CONDITION(std::abs(meanObject - mean) < mitk::eps, "calculated mean: " << meanObject << " expected mean: " << mean);
    // in one test case MPP is None because the roi has no positive pixels
    if (!std::isnan(mppObject))
    {
        MITK_TEST_CONDITION(std::abs(mppObject - MPP) < mitk::eps, "calculated MPP: " << mppObject << " expected MPP: " << MPP);
    }
    MITK_TEST_CONDITION(std::abs(medianObject - median) < mitk::eps, "calculated median: " << medianObject << " expected median: " << median);
    MITK_TEST_CONDITION(std::abs(skewnessObject - skewness) < mitk::eps, "calculated skewness: " << skewnessObject << " expected skewness: " << skewness);
    MITK_TEST_CONDITION(std::abs(kurtosisObject - kurtosis) < mitk::eps, "calculated kurtosis: " << kurtosisObject << " expected kurtosis: " << kurtosis);
    MITK_TEST_CONDITION(std::abs(uniformityObject - uniformity) < mitk::eps, "calculated uniformity: " << uniformityObject << " expected uniformity: " << uniformity);
    MITK_TEST_CONDITION(std::abs(uppObject - UPP) < mitk::eps, "calculated UPP: " << uppObject << " expected UPP: " << UPP);
    MITK_TEST_CONDITION(std::abs(varianceObject - variance) < mitk::eps, "calculated variance: " << varianceObject << " expected variance: " << variance);
    MITK_TEST_CONDITION(std::abs(standardDeviationObject - stdev) < mitk::eps, "calculated stdev: " << standardDeviationObject << " expected stdev: " << stdev);
    MITK_TEST_CONDITION(std::abs(minObject - min) < mitk::eps, "calculated min: " << minObject << " expected min: " << min);
    MITK_TEST_CONDITION(std::abs(maxObject - max) < mitk::eps, "calculated max: " << maxObject << " expected max: " << max);
    MITK_TEST_CONDITION(std::abs(rmsObject - RMS) < mitk::eps, "calculated RMS: " << rmsObject << " expected RMS: " << RMS);
    MITK_TEST_CONDITION(std::abs(entropyObject - entropy) < mitk::eps, "calculated entropy: " << entropyObject << " expected entropy: " << entropy);
    for (unsigned int i = 0; i < minIndex.size(); ++i)
    {
        MITK_TEST_CONDITION(std::abs(minIndexObject[i] - minIndex[i]) < mitk::eps, "minIndex [" << i << "] = " << minIndexObject[i] << " expected: " << minIndex[i]);
    }
    for (unsigned int i = 0; i < maxIndex.size(); ++i)
    {
        MITK_TEST_CONDITION(std::abs(maxIndexObject[i] - maxIndex[i]) < mitk::eps, "maxIndex [" << i << "] = " << maxIndexObject[i] << " expected: " << maxIndex[i]);
    }
}

void mitkImageStatisticsCalculatorTestSuite::TestUninitializedImage()
{
  /*****************************
  * loading uninitialized image to datastorage
  ******************************/
  MITK_INFO << std::endl << "Test uninitialized image: -----------------------------------------------------------------------------------";
  MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception)
  mitk::Image::Pointer image = mitk::Image::New();
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(image);

  mitk::ImageStatisticsCalculator::Pointer is = mitk::ImageStatisticsCalculator::New();
  is->GetStatistics();
  MITK_TEST_FOR_EXCEPTION_END(mitk::Exception)
}

MITK_TEST_SUITE_REGISTRATION(mitkImageStatisticsCalculator)
