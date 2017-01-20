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
  MITK_TEST(TestImageMaskingEmpty);
  MITK_TEST(TestImageMaskingNonEmpty);
  MITK_TEST(TestRecomputeOnModifiedMask);
  MITK_TEST(TestPic3DStatistics);
  MITK_TEST(TestPic3DAxialPlanarFigureMaskStatistics);
  MITK_TEST(TestPic3DSagittalPlanarFigureMaskStatistics);
  MITK_TEST(TestPic3DCoronalPlanarFigureMaskStatistics);
  MITK_TEST(TestPic3DImageMaskStatistics_label1);
  MITK_TEST(TestPic3DImageMaskStatistics_label2);
  MITK_TEST(TestPic3DIgnorePixelValueMaskStatistics);
  MITK_TEST(TestPic3DSecondaryMaskStatistics);
  MITK_TEST(TestUS4DCylStatistics_time1);
  MITK_TEST(TestUS4DCylAxialPlanarFigureMaskStatistics_time1);
  MITK_TEST(TestUS4DCylSagittalPlanarFigureMaskStatistics_time1);
  MITK_TEST(TestUS4DCylCoronalPlanarFigureMaskStatistics_time1);
  MITK_TEST(TestUS4DCylImageMaskStatistics_time1_label_1);
  MITK_TEST(TestUS4DCylImageMaskStatistics_time2_label_1);
  MITK_TEST(TestUS4DCylImageMaskStatistics_time1_label_2);
  MITK_TEST(TestUS4DCylIgnorePixelValueMaskStatistics_time1);
  MITK_TEST(TestUS4DCylSecondaryMaskStatistics_time1);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override;
  void tearDown() override;

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
  void TestImageMaskingEmpty();
  void TestImageMaskingNonEmpty();
  void TestRecomputeOnModifiedMask();

  void TestPic3DStatistics();
  void TestPic3DAxialPlanarFigureMaskStatistics();
  void TestPic3DSagittalPlanarFigureMaskStatistics();
  void TestPic3DCoronalPlanarFigureMaskStatistics();
  void TestPic3DImageMaskStatistics_label1();
  void TestPic3DImageMaskStatistics_label2();
  void TestPic3DIgnorePixelValueMaskStatistics();
  void TestPic3DSecondaryMaskStatistics();

  void TestUS4DCylStatistics_time1();
  void TestUS4DCylAxialPlanarFigureMaskStatistics_time1();
  void TestUS4DCylSagittalPlanarFigureMaskStatistics_time1();
  void TestUS4DCylCoronalPlanarFigureMaskStatistics_time1();
  void TestUS4DCylImageMaskStatistics_time1_label_1();
  void TestUS4DCylImageMaskStatistics_time2_label_1();
  void TestUS4DCylImageMaskStatistics_time1_label_2();
  void TestUS4DCylIgnorePixelValueMaskStatistics_time1();
  void TestUS4DCylSecondaryMaskStatistics_time1();

  void TestDifferentNBinsForHistogramStatistics();
  void TestDifferentBinSizeForHistogramStatistic();

  void TestSwitchFromBinSizeToNBins();
  void TestSwitchFromNBinsToBinSize();

private:

  mitk::Image::Pointer m_TestImage;

  mitk::Image::Pointer m_Pic3DImage;
  mitk::Image::Pointer m_Pic3DImageMask;
  mitk::Image::Pointer m_Pic3DImageMask2;
  mitk::PlanarFigure::Pointer m_Pic3DPlanarFigureAxial;
  mitk::PlanarFigure::Pointer m_Pic3DPlanarFigureSagittal;
  mitk::PlanarFigure::Pointer m_Pic3DPlanarFigureCoronal;

  mitk::Image::Pointer m_US4DImage;
  mitk::Image::Pointer m_US4DImageMask;
  mitk::Image::Pointer m_US4DImageMask2;
  mitk::PlanarFigure::Pointer m_US4DPlanarFigureAxial;
  mitk::PlanarFigure::Pointer m_US4DPlanarFigureSagittal;
  mitk::PlanarFigure::Pointer m_US4DPlanarFigureCoronal;

  mitk::PlaneGeometry::Pointer m_Geometry;

  // calculate statistics for the given image and planarpolygon
  const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer ComputeStatistics( mitk::Image::Pointer image,
                                                                       mitk::PlanarFigure::Pointer polygon );

  // calculate statistics for the given image and mask
  const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer ComputeStatistics( mitk::Image::Pointer image,
                                                                       mitk::Image::Pointer image_mask );

  // universal function to calculate statistics
  const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer ComputeStatisticsNew(mitk::Image::Pointer image,
                                                                                           int timeStep=0,
                                                                                           mitk::MaskGenerator::Pointer maskGen=nullptr,
                                                                                           mitk::MaskGenerator::Pointer secondardMaskGen=nullptr,
                                                                                           unsigned short label=1);

  void VerifyStatistics(mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer stats,
                        double testMean, double testSD, double testMedian=0);

  void VerifyStatistics(mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer stats,
                        long N,
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

void mitkImageStatisticsCalculatorTestSuite::tearDown()
{
    m_TestImage = nullptr;

    m_Pic3DImage = nullptr;
    m_Pic3DImageMask = nullptr;
    m_Pic3DImageMask2 = nullptr;
    m_Pic3DPlanarFigureAxial = nullptr;
    m_Pic3DPlanarFigureSagittal = nullptr;
    m_Pic3DPlanarFigureCoronal = nullptr;

    m_US4DImage = nullptr;
    m_US4DImageMask = nullptr;
    m_US4DImageMask2 = nullptr;
    m_US4DPlanarFigureAxial = nullptr;
    m_US4DPlanarFigureSagittal = nullptr;
    m_US4DPlanarFigureCoronal = nullptr;

    m_Geometry = nullptr;
}

void mitkImageStatisticsCalculatorTestSuite::setUp()
{
    std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");

    std::string Pic3DFile = this->GetTestDataFilePath("Pic3D.nrrd");
    std::string Pic3DImageMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D-labels.nrrd");
    std::string Pic3DImageMaskFile2 = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D-labels2.nrrd");
    std::string Pic3DAxialPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3DAxialPlanarFigure.pf");
    std::string Pic3DSagittalPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3DSagittalPlanarFigure.pf");
    std::string Pic3DCoronalPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3DCoronalPlanarFigure.pf");

    std::string US4DFile = this->GetTestDataFilePath("US4DCyl.nrrd");
    std::string US4DImageMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D-labels.nrrd");
    std::string US4DImageMaskFile2 = this->GetTestDataFilePath("ImageStatisticsTestData/US4D-labels2.nrrd");
    std::string US4DAxialPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4DAxialPlanarFigure.pf");
    std::string US4DSagittalPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4DSagittalPlanarFigure.pf");
    std::string US4DCoronalPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4DCoronalPlanarFigure.pf");

  if (filename.empty() ||
          Pic3DFile.empty() || Pic3DImageMaskFile.empty() ||
          Pic3DAxialPlanarFigureFile.empty() || Pic3DSagittalPlanarFigureFile.empty() || Pic3DCoronalPlanarFigureFile.empty() ||
          US4DFile.empty() || US4DImageMaskFile.empty() ||
          US4DAxialPlanarFigureFile.empty() || US4DSagittalPlanarFigureFile.empty() || US4DCoronalPlanarFigureFile.empty())
  {
    MITK_TEST_FAILED_MSG( << "Could not find test file" )
  }

  MITK_TEST_OUTPUT(<< "Loading test image '" << filename << "'")

  m_TestImage = mitk::IOUtil::LoadImage(filename);
  MITK_TEST_CONDITION_REQUIRED( m_TestImage.IsNotNull(), "Loaded an mitk::Image" );

  m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED( m_Geometry.IsNotNull(), "Getting image geometry" );

  m_Pic3DImage = mitk::IOUtil::LoadImage(Pic3DFile);
  MITK_TEST_CONDITION_REQUIRED( m_Pic3DImage.IsNotNull(), "Loaded Pic3D" );
  m_Pic3DImageMask = mitk::IOUtil::LoadImage(Pic3DImageMaskFile);
  MITK_TEST_CONDITION_REQUIRED( m_Pic3DImageMask.IsNotNull(), "Loaded Pic3D image mask" );
  m_Pic3DImageMask2 = mitk::IOUtil::LoadImage(Pic3DImageMaskFile2);
  MITK_TEST_CONDITION_REQUIRED( m_Pic3DImageMask2.IsNotNull(), "Loaded Pic3D image secondary mask" );
  m_Pic3DPlanarFigureAxial = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::Load(Pic3DAxialPlanarFigureFile)[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED( m_Pic3DPlanarFigureAxial.IsNotNull(), "Loaded Pic3D axial planarFigure" );
  m_Pic3DPlanarFigureSagittal = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::Load(Pic3DSagittalPlanarFigureFile)[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED( m_Pic3DPlanarFigureSagittal.IsNotNull(), "Loaded Pic3D sagittal planarFigure" );
  m_Pic3DPlanarFigureCoronal = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::Load(Pic3DCoronalPlanarFigureFile)[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED( m_Pic3DPlanarFigureCoronal.IsNotNull(), "Loaded Pic3D coronal planarFigure" );

  m_US4DImage = mitk::IOUtil::LoadImage(US4DFile);
  MITK_TEST_CONDITION_REQUIRED( m_US4DImage.IsNotNull(), "Loaded US4D" );
  m_US4DImageMask = mitk::IOUtil::LoadImage(US4DImageMaskFile);
  MITK_TEST_CONDITION_REQUIRED( m_US4DImageMask.IsNotNull(), "Loaded US4D image mask" );
  m_US4DImageMask2 = mitk::IOUtil::LoadImage(US4DImageMaskFile2);
  MITK_TEST_CONDITION_REQUIRED( m_US4DImageMask2.IsNotNull(), "Loaded US4D image mask2" );
  m_US4DPlanarFigureAxial = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::Load(US4DAxialPlanarFigureFile)[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED( m_US4DPlanarFigureAxial.IsNotNull(), "Loaded US4D axial planarFigure" );
  m_US4DPlanarFigureSagittal = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::Load(US4DSagittalPlanarFigureFile)[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED( m_US4DPlanarFigureSagittal.IsNotNull(), "Loaded US4D sagittal planarFigure" );
  m_US4DPlanarFigureCoronal = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::Load(US4DCoronalPlanarFigureFile)[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED( m_US4DPlanarFigureCoronal.IsNotNull(), "Loaded US4D coronal planarFigure" );

}

void mitkImageStatisticsCalculatorTestSuite::TestCase1()
{
  /*****************************
   * one whole white pixel
   * -> mean of 255 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 1:-----------------------------------------------------------------------------------";
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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 255.0, 0.0, 255.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase2()
{
  /*****************************
   * half pixel in x-direction (white)
   * -> mean of 255 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 2:-----------------------------------------------------------------------------------";

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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 255.0, 0.0, 255.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase3()
{
  /*****************************
   * half pixel in diagonal-direction (white)
   * -> mean of 255 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 3:-----------------------------------------------------------------------------------";
  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 10.5 ; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  figure1->GetPolyLine(0);

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 255.0, 0.0, 255.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase4()
{
  /*****************************
   * one pixel (white) + 2 half pixels (white) + 1 half pixel (black)
   * -> mean of 191.25 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 4:-----------------------------------------------------------------------------------";
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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 191.25, 110.41, 242.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase5()
{
  /*****************************
   * whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 191.5 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 5:-----------------------------------------------------------------------------------";
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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 191.50, 63.50, 134.340);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase6()
{
  /*****************************
   * quarter pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 191.5 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 6:-----------------------------------------------------------------------------------";
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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 191.5, 63.50, 134.340);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase7()
{
  /*****************************
   * half pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 127.66 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 7:-----------------------------------------------------------------------------------";

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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure1.GetPointer()), 127.66, 104.1, 140.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase8()
{
  /*****************************
   * whole pixel (gray)
   * -> mean of 128 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 8:-----------------------------------------------------------------------------------";

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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure2.GetPointer()), 128.0, 0.0, 128.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase9()
{
  /*****************************
   * whole pixel (gray) + half pixel (white) in y-direction
   * -> mean of 191.5 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 9:-----------------------------------------------------------------------------------";

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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure2.GetPointer()), 191.5, 63.50, 134.340);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase10()
{
  /*****************************
   * 2 whole pixel (white) + 2 whole pixel (black) in y-direction
   * -> mean of 127.66 expected
   ******************************/
    MITK_INFO << std::endl << "Test case 10:-----------------------------------------------------------------------------------";

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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure2.GetPointer()), 127.66, 104.1, 140.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase11()
{
  /*****************************
   * 9 whole pixels (white) + 3 half pixels (white)
   * + 3 whole pixel (black) [ + 3 slightly less than half pixels (black)]
   * -> mean of 204.0 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 11:-----------------------------------------------------------------------------------";

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

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure2.GetPointer()), 204.0, 102.00, 242.250);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase12()
{
  /*****************************
   * half pixel (white) + whole pixel (white) + half pixel (black)
   * -> mean of 212.66 expected
   ******************************/
  MITK_INFO << std::endl << "Test case 12:-----------------------------------------------------------------------------------";

  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 9.5; pnt1[1] = 0.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 2.5;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 11.5; pnt3[1] = 2.5;
  figure2->SetControlPoint( 2, pnt3, true );
  figure2->GetPolyLine(0);

  this->VerifyStatistics(ComputeStatistics(m_TestImage, figure2.GetPointer()), 212.66, 59.860, 248.640);
}

void mitkImageStatisticsCalculatorTestSuite::TestImageMaskingEmpty()
{
  MITK_INFO << std::endl << "TestImageMaskingEmpty:-----------------------------------------------------------------------------------";

  mitk::Image::Pointer mask_image = mitk::ImageGenerator::GenerateImageFromReference<unsigned char>( m_TestImage, 0 );

  this->VerifyStatistics( ComputeStatistics( m_TestImage, mask_image ), -21474836.480, -21474836.480, -21474836.480); // empty statisticsContainer (default values)
}

void mitkImageStatisticsCalculatorTestSuite::TestImageMaskingNonEmpty()
{
  MITK_INFO << std::endl << "TestImageMaskingNonEmpty:-----------------------------------------------------------------------------------";

  mitk::Image::Pointer mask_image = mitk::ImageGenerator::GenerateImageFromReference<unsigned char>( m_TestImage, 0 );

  std::vector< itk::Index<3U> > activated_indices;
  itk::Index<3U> index = {{10, 8, 0}};
  activated_indices.push_back( index );

  index[0] = 9;  index[1] = 8; index[2] =  0;
  activated_indices.push_back( index );

  index[0] = 9;  index[1] = 7; index[2] =  0;
  activated_indices.push_back( index );

  index[0] = 10;  index[1] = 7; index[2] =  0;
  activated_indices.push_back( index );

  std::vector< itk::Index<3U> >::const_iterator indexIter = activated_indices.begin();

  // activate voxel in the mask image
  mitk::ImagePixelWriteAccessor< unsigned char, 3> writeAccess( mask_image );
  while( indexIter != activated_indices.end() )
  {
    writeAccess.SetPixelByIndex( (*indexIter++), 1);
  }

  this->VerifyStatistics( ComputeStatistics( m_TestImage, mask_image ), 127.5, 127.5, 12.750);
}

void mitkImageStatisticsCalculatorTestSuite::TestRecomputeOnModifiedMask()
{
  MITK_INFO << std::endl << "TestRecomputeOnModifiedMask:-----------------------------------------------------------------------------------";
  mitk::Image::Pointer mask_image = mitk::ImageGenerator::GenerateImageFromReference<unsigned char>( m_TestImage, 0 );

  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
  statisticsCalculator->SetInputImage( m_TestImage );

  mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
  imgMaskGen->SetImageMask(mask_image);

  statisticsCalculator->SetMask(imgMaskGen.GetPointer());

  this->VerifyStatistics( statisticsCalculator->GetStatistics(), -21474836.480, -21474836.480, -21474836.480);

  // activate voxel in the mask image
  itk::Index<3U> test_index = {11, 8, 0};
  mitk::ImagePixelWriteAccessor< unsigned char, 3> writeAccess( mask_image );
  writeAccess.SetPixelByIndex( test_index, 1);

  mask_image->Modified();

  mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer stat = statisticsCalculator->GetStatistics();

  this->VerifyStatistics( stat, 128.0, 0.0, 128.0);
  MITK_TEST_CONDITION( stat->GetN() == 1, "Calculated mask voxel count '" << stat->GetN() << "'  is equal to the desired value '" << 1 << "'" );

}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DStatistics()
{
    MITK_INFO << std::endl << "Test plain Pic3D:-----------------------------------------------------------------------------------";
    long expected_N = 3211264;
    double expected_mean = -365.80015345982144;
    double expected_MPP = 111.80226129535752;
    double expected_median = -105.16000366210938;
    double expected_skewness = -0.26976612134147004;
    double expected_kurtosis = 1.4655017209571437;
    double expected_uniformity = 0.06087994379480554;
    double expected_UPP = 0.011227934437026977;
    double expected_variance = 224036.80150510342;
    double expected_standarddev = 473.32525973700518;
    double expected_min = -1023;
    double expected_max = 1361;
    double expected_RMS = 598.20276978323352;
    double expected_entropy = 4.6727423654570357;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 0;
    expected_minIndex[1] = 0;
    expected_minIndex[2] = 0;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 139;
    expected_maxIndex[1] = 182;
    expected_maxIndex[2] = 43;

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DAxialPlanarFigureMaskStatistics()
{
    MITK_INFO << std::endl << "Test Pic3D axial pf:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.6719817476387417;
    double expected_kurtosis = 5.8846935191205221;
    double expected_MPP = 230.43933685003768;
    double expected_max = 1206;
    double expected_mean = 182.30282131661443;
    double expected_median = 95.970001220703125;
    double expected_min = -156;
    long expected_N = 3190;
    double expected_RMS = 301.93844376702253;
    double expected_skewness = 1.6400489794326298;
    double expected_standarddev = 240.69172225993557;
    double expected_UPP = 0.024889790784288681;
    double expected_uniformity = 0.027579917650180332;
    double expected_variance = 57932.505164453964;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 156;
    expected_minIndex[1] = 133;
    expected_minIndex[2] = 24;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 125;
    expected_maxIndex[1] = 167;
    expected_maxIndex[2] = 24;

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetInputImage(m_Pic3DImage);
    pfMaskGen->SetPlanarFigure(m_Pic3DPlanarFigureAxial);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, pfMaskGen.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DSagittalPlanarFigureMaskStatistics()
{
    MITK_INFO << std::endl << "Test Pic3D sagittal pf:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.6051911962074286;
    double expected_kurtosis = 6.5814062739142338;
    double expected_MPP = 249.03202846975088;
    double expected_max = 1240;
    double expected_mean = 233.93602693602693;
    double expected_median = 174.9849853515625;
    double expected_min = -83;
    long expected_N = 1188;
    double expected_RMS = 332.03230188484594;
    double expected_skewness = 1.7489809015501814;
    double expected_standarddev = 235.62551813489128;
    double expected_UPP = 0.026837539253364174;
    double expected_uniformity = 0.027346982734188126;
    double expected_variance = 55519.384796335973;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 128;
    expected_minIndex[1] = 119;
    expected_minIndex[2] = 22;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 128;
    expected_maxIndex[1] = 167;
    expected_maxIndex[2] = 22;

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetInputImage(m_Pic3DImage);
    pfMaskGen->SetPlanarFigure(m_Pic3DPlanarFigureSagittal);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, pfMaskGen.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DCoronalPlanarFigureMaskStatistics()
{
    MITK_INFO << std::endl << "Test Pic3D coronal pf:-----------------------------------------------------------------------------------";

    double expected_entropy = 6.0677398647867449;
    double expected_kurtosis = 1.6242929941303372;
    double expected_MPP = 76.649350649350652;
    double expected_max = 156;
    double expected_mean = -482.14807692307693;
    double expected_median = -660.07501220703125;
    double expected_min = -897;
    long expected_N = 520;
    double expected_RMS = 595.09446729069839;
    double expected_skewness = 0.51691492278851858;
    double expected_standarddev = 348.81321207686312;
    double expected_UPP = 0.0021560650887573964;
    double expected_uniformity = 0.020295857988165685;
    double expected_variance = 121670.6569193787;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 217;
    expected_minIndex[1] = 127;
    expected_minIndex[2] = 43;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 209;
    expected_maxIndex[1] = 127;
    expected_maxIndex[2] = 39;

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetInputImage(m_Pic3DImage);
    pfMaskGen->SetPlanarFigure(m_Pic3DPlanarFigureCoronal);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, pfMaskGen.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DImageMaskStatistics_label1()
{
    MITK_INFO << std::endl << "Test Pic3D image mask label 1 pf:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.695858251095868;
    double expected_kurtosis = 4.2728827997815717;
    double expected_MPP = 413.52408256880733;
    double expected_max = 1206;
    double expected_mean = 413.52408256880733;
    double expected_median = 324;
    double expected_min = 6;
    long expected_N = 872;
    double expected_RMS = 472.02024695145235;
    double expected_skewness = 1.3396074364415382;
    double expected_standarddev = 227.59821323493802;
    double expected_UPP = 0.029758648261930806;
    double expected_uniformity = 0.029758648261930806;
    double expected_variance = 51800.946667736309;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 135;
    expected_minIndex[1] = 158;
    expected_minIndex[2] = 24;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 125;
    expected_maxIndex[1] = 167;
    expected_maxIndex[2] = 24;

    mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
    imgMaskGen->SetImageMask(m_Pic3DImageMask);
    imgMaskGen->SetInputImage(m_Pic3DImage);
    imgMaskGen->SetTimeStep(0);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, imgMaskGen.GetPointer(), nullptr, 1);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestPic3DImageMaskStatistics_label2()
{
    MITK_INFO << std::endl << "Test Pic3D image mask label 2 pf:-----------------------------------------------------------------------------------";

    double expected_entropy = 4.3685781901212764;
    double expected_kurtosis = 9.7999112757587934;
    double expected_MPP = -nan("");
    double expected_max = -145;
    double expected_mean = -897.92833876221493;
    double expected_median = -969.16499900817871;
    double expected_min = -1008;
    long expected_N = 307;
    double expected_RMS = 913.01496468179471;
    double expected_skewness = 2.6658524648889736;
    double expected_standarddev = 165.29072623903585;
    double expected_UPP = 0;
    double expected_uniformity = 0.087544695434434425;
    double expected_variance = 27321.024180627897;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 170;
    expected_minIndex[1] = 60;
    expected_minIndex[2] = 24;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 173;
    expected_maxIndex[1] = 57;
    expected_maxIndex[2] = 24;

    mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
    imgMaskGen->SetImageMask(m_Pic3DImageMask);
    imgMaskGen->SetInputImage(m_Pic3DImage);
    imgMaskGen->SetTimeStep(0);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, imgMaskGen.GetPointer(), nullptr, 2);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestPic3DIgnorePixelValueMaskStatistics()
{
    MITK_INFO << std::endl << "Test Pic3D ignore zero pixels:-----------------------------------------------------------------------------------";

    double expected_entropy = 4.671045011438645;
    double expected_kurtosis = 1.4638176488404484;
    double expected_MPP = 111.80226129535752;
    double expected_max = 1361;
    double expected_mean = -366.48547402877585;
    double expected_median = -105.16000366210938;
    double expected_min = -1023;
    long expected_N = 3205259;
    double expected_RMS = 598.76286909522139;
    double expected_skewness = -0.26648854845130782;
    double expected_standarddev = 473.50329537717545;
    double expected_UPP = 0.011270044547276429;
    double expected_uniformity = 0.061029773286547614;
    double expected_variance = 224205.37073304466;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 0;
    expected_minIndex[1] = 0;
    expected_minIndex[2] = 0;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 139;
    expected_maxIndex[1] = 182;
    expected_maxIndex[2] = 43;

    mitk::IgnorePixelMaskGenerator::Pointer ignPixelValMask = mitk::IgnorePixelMaskGenerator::New();
    ignPixelValMask->SetInputImage(m_Pic3DImage);
    ignPixelValMask->SetIgnoredPixelValue(0);
    ignPixelValMask->SetTimeStep(0);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, ignPixelValMask.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestPic3DSecondaryMaskStatistics()
{
    MITK_INFO << std::endl << "Test Pic3D ignore zero pixels AND Image mask 2:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.9741637167320176;
    double expected_kurtosis = 3.490663358061596;
    double expected_MPP = 332.43534482758622;
    double expected_max = 1206;
    double expected_mean = 320.63333333333333;
    double expected_median = 265.06500244140625;
    double expected_min = -57;
    long expected_N = 720;
    double expected_RMS = 433.57749531594055;
    double expected_skewness = 1.1047775627624981;
    double expected_standarddev = 291.86248474238687;
    double expected_UPP = 0.020628858024691339;
    double expected_uniformity = 0.021377314814814797;
    double expected_variance = 85183.710000000006;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 116;
    expected_minIndex[1] = 170;
    expected_minIndex[2] = 24;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 125;
    expected_maxIndex[1] = 167;
    expected_maxIndex[2] = 24;

    mitk::IgnorePixelMaskGenerator::Pointer ignPixelValMask = mitk::IgnorePixelMaskGenerator::New();
    ignPixelValMask->SetInputImage(m_Pic3DImage);
    ignPixelValMask->SetIgnoredPixelValue(0);
    ignPixelValMask->SetTimeStep(0);

    mitk::ImageMaskGenerator::Pointer imgMaskGen2 = mitk::ImageMaskGenerator::New();
    imgMaskGen2->SetImageMask(m_Pic3DImageMask2);
    imgMaskGen2->SetInputImage(m_Pic3DImage);
    imgMaskGen2->SetTimeStep(0);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_Pic3DImage, 0, imgMaskGen2.GetPointer(), ignPixelValMask.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylStatistics_time1()
{
    MITK_INFO << std::endl << "Test plain US4D timeStep1:-----------------------------------------------------------------------------------";

    double expected_entropy = 4.8272774900452502;
    double expected_kurtosis = 6.1336513352934432;
    double expected_MPP = 53.395358640738536;
    double expected_max = 199;
    double expected_mean = 35.771298153622375;
    double expected_median = 20.894999504089355;
    double expected_min = 0;
    long expected_N = 3409920;
    double expected_RMS = 59.244523377028408;
    double expected_skewness = 1.8734292240015058;
    double expected_standarddev = 47.226346233600559;
    double expected_UPP = 0.12098731125004937;
    double expected_uniformity = 0.12098731125004937;
    double expected_variance = 2230.3277785759178;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 0;
    expected_minIndex[1] = 0;
    expected_minIndex[2] = 0;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 268;
    expected_maxIndex[1] = 101;
    expected_maxIndex[2] = 0;

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylAxialPlanarFigureMaskStatistics_time1()
{
    MITK_INFO << std::endl << "Test US4D axial pf timeStep1:-----------------------------------------------------------------------------------";

    double expected_entropy = 6.218151288002292;
    double expected_kurtosis = 1.7322676370242023;
    double expected_MPP = 121.11663807890223;
    double expected_max = 199;
    double expected_mean = 121.11663807890223;
    double expected_median = 120.14999771118164;
    double expected_min = 9;
    long expected_N = 2332;
    double expected_RMS = 134.41895158590751;
    double expected_skewness = -0.1454808104597369;
    double expected_standarddev = 58.30278317472294;
    double expected_UPP = 0.021354765820606133;
    double expected_uniformity = 0.021354765820606133;
    double expected_variance = 3399.214525918756;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 129;
    expected_minIndex[1] = 131;
    expected_minIndex[2] = 19;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 126;
    expected_maxIndex[1] = 137;
    expected_maxIndex[2] = 19;

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetInputImage(m_US4DImage);
    pfMaskGen->SetPlanarFigure(m_US4DPlanarFigureAxial);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, pfMaskGen.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylSagittalPlanarFigureMaskStatistics_time1()
{
    MITK_INFO << std::endl << "Test US4D sagittal pf timeStep1:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.2003987046387508;
    double expected_kurtosis = 2.7574491062430142;
    double expected_MPP = 26.212534059945504;
    double expected_max = 59;
    double expected_mean = 26.176870748299319;
    double expected_median = 26.254999160766602;
    double expected_min = 0;
    long expected_N = 735;
    double expected_RMS = 28.084905283121476;
    double expected_skewness = 0.18245181360752327;
    double expected_standarddev = 10.175133541567705;
    double expected_UPP = 0.032921467906890628;
    double expected_uniformity = 0.032921467906890628;
    double expected_variance = 103.53334258873615;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 147;
    expected_minIndex[1] = 94;
    expected_minIndex[2] = 21;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 147;
    expected_maxIndex[1] = 77;
    expected_maxIndex[2] = 24;

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetInputImage(m_US4DImage);
    pfMaskGen->SetPlanarFigure(m_US4DPlanarFigureSagittal);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, pfMaskGen.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylCoronalPlanarFigureMaskStatistics_time1()
{
    MITK_INFO << std::endl << "Test US4D coronal pf timeStep1:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.8892941136639161;
    double expected_kurtosis = 4.6434920707409564;
    double expected_MPP = 55.486426346239433;
    double expected_max = 199;
    double expected_mean = 55.118479221927501;
    double expected_median = 36.815000534057617;
    double expected_min = 0;
    long expected_N = 2262;
    double expected_RMS = 71.98149752438627;
    double expected_skewness = 1.4988288344523237;
    double expected_standarddev = 46.29567187238105;
    double expected_UPP = 0.023286748110675673;
    double expected_uniformity = 0.023286748110675673;
    double expected_variance = 2143.2892341151742;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 214;
    expected_minIndex[1] = 169;
    expected_minIndex[2] = 10;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 99;
    expected_maxIndex[1] = 169;
    expected_maxIndex[2] = 17;

    mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
    pfMaskGen->SetInputImage(m_US4DImage);
    pfMaskGen->SetPlanarFigure(m_US4DPlanarFigureCoronal);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, pfMaskGen.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylImageMaskStatistics_time1_label_1()
{
    MITK_INFO << std::endl << "Test US4D image mask time 1 label 1:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.0082903903398677;
    double expected_kurtosis = 3.6266994778237809;
    double expected_MPP = 169.58938547486034;
    double expected_max = 199;
    double expected_mean = 169.58938547486034;
    double expected_median = 187.44000244140625;
    double expected_min = 63;
    long expected_N = 716;
    double expected_RMS = 173.09843164831432;
    double expected_skewness = -1.2248969838579555;
    double expected_standarddev = 34.677188083311712;
    double expected_UPP = 0.076601073624418703;
    double expected_uniformity = 0.076601073624418703;
    double expected_variance = 1202.5073733653758;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 82;
    expected_minIndex[1] = 158;
    expected_minIndex[2] = 19;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 126;
    expected_maxIndex[1] = 140;
    expected_maxIndex[2] = 19;

    mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
    imgMask1->SetInputImage(m_US4DImage);
    imgMask1->SetImageMask(m_US4DImageMask);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, imgMask1.GetPointer(), nullptr, 1);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylImageMaskStatistics_time2_label_1()
{
    MITK_INFO << std::endl << "Test US4D image mask time 2 label 1:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.1857604214916506;
    double expected_kurtosis = 3.0692303858330683;
    double expected_MPP = 167.97194163860831;
    double expected_max = 199;
    double expected_mean = 167.97194163860831;
    double expected_median = 184.39499664306641;
    double expected_min = 72;
    long expected_N = 891;
    double expected_RMS = 171.67986611998634;
    double expected_skewness = -1.1221651136259736;
    double expected_standarddev = 35.488071983870803;
    double expected_UPP = 0.063124070232188439;
    double expected_uniformity = 0.063124070232188439;
    double expected_variance = 1259.4032531323958;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 103;
    expected_minIndex[1] = 212;
    expected_minIndex[2] = 19;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 102;
    expected_maxIndex[1] = 168;
    expected_maxIndex[2] = 19;

    mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
    imgMask1->SetInputImage(m_US4DImage);
    imgMask1->SetImageMask(m_US4DImageMask);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 2, imgMask1.GetPointer(), nullptr, 1);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylImageMaskStatistics_time1_label_2()
{
    MITK_INFO << std::endl << "Test US4D image mask time 1 label 2:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.0822234230119001;
    double expected_kurtosis = 2.4346603343623747;
    double expected_MPP = 20.733626373626375;
    double expected_max = 46;
    double expected_mean = 20.624836029733274;
    double expected_median = 20.010000228881836;
    double expected_min = 0;
    long expected_N = 2287;
    double expected_RMS = 22.508347574573804;
    double expected_skewness = 0.13837218490626488;
    double expected_standarddev = 9.0134260569684965;
    double expected_UPP = 0.034783970308787;
    double expected_uniformity = 0.034783970308787;
    double expected_variance = 81.241849284438644;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 178;
    expected_minIndex[1] = 76;
    expected_minIndex[2] = 19;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 198;
    expected_maxIndex[1] = 90;
    expected_maxIndex[2] = 19;

    mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
    imgMask1->SetInputImage(m_US4DImage);
    imgMask1->SetImageMask(m_US4DImageMask);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, imgMask1.GetPointer(), nullptr, 2);
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylIgnorePixelValueMaskStatistics_time1()
{
    MITK_INFO << std::endl << "Test US4D ignore zero pixels:-----------------------------------------------------------------------------------";

    double expected_entropy = 5.8609813848087962;
    double expected_kurtosis = 4.7556214582883651;
    double expected_MPP = 53.395358640738536;
    double expected_max = 199;
    double expected_mean = 53.395358640738536;
    double expected_median = 35.649999618530273;
    double expected_min = 1;
    long expected_N = 2284417;
    double expected_RMS = 72.382339046507084;
    double expected_skewness = 1.588289859859108;
    double expected_standarddev = 48.868585834566694;
    double expected_UPP = 0.023927063695115193;
    double expected_uniformity = 0.023927063695115193;
    double expected_variance = 2388.1386814704128;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 187;
    expected_minIndex[1] = 19;
    expected_minIndex[2] = 0;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 268;
    expected_maxIndex[1] = 101;
    expected_maxIndex[2] = 0;

    mitk::IgnorePixelMaskGenerator::Pointer ignPixelValMask = mitk::IgnorePixelMaskGenerator::New();
    ignPixelValMask->SetInputImage(m_US4DImage);
    ignPixelValMask->SetIgnoredPixelValue(0);
    ignPixelValMask->SetTimeStep(1);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, ignPixelValMask.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


void mitkImageStatisticsCalculatorTestSuite::TestUS4DCylSecondaryMaskStatistics_time1()
{
    MITK_INFO << std::endl << "Test US4d ignore zero pixels AND Image mask 2:-----------------------------------------------------------------------------------";

    double expected_entropy = 4.9955858614274558;
    double expected_kurtosis = 17.471042803365179;
    double expected_MPP = 32.791403286978507;
    double expected_max = 199;
    double expected_mean = 32.791403286978507;
    double expected_median = 25.75;
    double expected_min = 1;
    long expected_N = 17402;
    double expected_RMS = 42.776697859745241;
    double expected_skewness = 3.3991813038552596;
    double expected_standarddev = 27.469433016621732;
    double expected_UPP = 0.043040554251756687;
    double expected_uniformity = 0.043040554251756687;
    double expected_variance = 754.56975025466807;
    vnl_vector<int> expected_minIndex;
    expected_minIndex.set_size(3);
    expected_minIndex[0] = 177;
    expected_minIndex[1] = 27;
    expected_minIndex[2] = 36;

    vnl_vector<int> expected_maxIndex;
    expected_maxIndex.set_size(3);
    expected_maxIndex[0] = 109;
    expected_maxIndex[1] = 116;
    expected_maxIndex[2] = 36;

    mitk::IgnorePixelMaskGenerator::Pointer ignPixelValMask = mitk::IgnorePixelMaskGenerator::New();
    ignPixelValMask->SetInputImage(m_US4DImage);
    ignPixelValMask->SetIgnoredPixelValue(0);

    mitk::ImageMaskGenerator::Pointer imgMaskGen2 = mitk::ImageMaskGenerator::New();
    imgMaskGen2->SetImageMask(m_US4DImageMask2);
    imgMaskGen2->SetInputImage(m_US4DImage);

    const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result = ComputeStatisticsNew(m_US4DImage, 1, imgMaskGen2.GetPointer(), ignPixelValMask.GetPointer());
    //std::cout << result->GetAsString();

    VerifyStatistics(result,
                     expected_N,
                     expected_mean,
                     expected_MPP,
                     expected_median,
                     expected_skewness,
                     expected_kurtosis,
                     expected_uniformity,
                     expected_UPP,
                     expected_variance,
                     expected_standarddev,
                     expected_min,
                     expected_max,
                     expected_RMS,
                     expected_entropy,
                     expected_minIndex,
                     expected_maxIndex);
}


const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatistics( mitk::Image::Pointer image, mitk::PlanarFigure::Pointer polygon )
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
  }

  return mitk::ImageStatisticsCalculator::StatisticsContainer::New();
}

const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatistics(mitk::Image::Pointer image, mitk::Image::Pointer image_mask )
{
  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
  statisticsCalculator->SetInputImage(image);

  statisticsCalculator->SetNBinsForHistogramStatistics(10);

  mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
  imgMaskGen->SetImageMask(image_mask);
  statisticsCalculator->SetMask(imgMaskGen.GetPointer());

  return statisticsCalculator->GetStatistics();
}

const mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatisticsNew(mitk::Image::Pointer image,
                                                             int timeStep,
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

    return imgStatCalc->GetStatistics(timeStep, label);
}

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer stats,
                                                              double testMean, double testSD, double testMedian)
{
  int tmpMean = stats->GetMean() * 100;
  double calculatedMean = tmpMean / 100.0;
  MITK_TEST_CONDITION( calculatedMean == testMean,
                       "Calculated mean grayvalue '" << calculatedMean <<
                       "'  is equal to the desired value '" << testMean << "'" );

  int tmpSD = stats->GetStd() * 100;
  double calculatedSD = tmpSD / 100.0;
  MITK_TEST_CONDITION( calculatedSD == testSD,
                       "Calculated grayvalue sd '" << calculatedSD <<
                       "'  is equal to the desired value '" << testSD <<"'" );

  int tmpMedian = stats->GetMedian() * 100;
  double calculatedMedian = tmpMedian / 100.0;
  MITK_TEST_CONDITION( testMedian == calculatedMedian,
                       "Calculated median grayvalue '" << calculatedMedian <<
                       "' is equal to the desired value '" << testMedian << "'");
}

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer stats,
                                                              long N,
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
    MITK_TEST_CONDITION(std::abs(stats->GetN() - N) < mitk::eps, "calculated N: " << stats->GetN() << " expected N: " << N);
    MITK_TEST_CONDITION(std::abs(stats->GetMean() - mean) < mitk::eps, "calculated mean: " << stats->GetMean() << " expected mean: " << mean);
    // in one test case MPP is None because the roi has no positive pixels
    if (!std::isnan(stats->GetMPP()))
    {
        MITK_TEST_CONDITION(std::abs(stats->GetMPP() - MPP) < mitk::eps, "calculated MPP: " << stats->GetMPP() << " expected MPP: " << MPP);
    }
    MITK_TEST_CONDITION(std::abs(stats->GetMedian() - median) < mitk::eps, "calculated median: " << stats->GetMedian() << " expected median: " << median);
    MITK_TEST_CONDITION(std::abs(stats->GetSkewness() - skewness) < mitk::eps, "calculated skewness: " << stats->GetSkewness() << " expected skewness: " << skewness);
    MITK_TEST_CONDITION(std::abs(stats->GetKurtosis() - kurtosis) < mitk::eps, "calculated kurtosis: " << stats->GetKurtosis() << " expected kurtosis: " << kurtosis);
    MITK_TEST_CONDITION(std::abs(stats->GetUniformity() - uniformity) < mitk::eps, "calculated uniformity: " << stats->GetUniformity() << " expected uniformity: " << uniformity);
    MITK_TEST_CONDITION(std::abs(stats->GetUPP() - UPP) < mitk::eps, "calculated UPP: " << stats->GetUPP() << " expected UPP: " << UPP);
    MITK_TEST_CONDITION(std::abs(stats->GetVariance() - variance) < mitk::eps, "calculated variance: " << stats->GetVariance() << " expected variance: " << variance);
    MITK_TEST_CONDITION(std::abs(stats->GetStd() - stdev) < mitk::eps, "calculated stdev: " << stats->GetStd() << " expected stdev: " << stdev);
    MITK_TEST_CONDITION(std::abs(stats->GetMin() - min) < mitk::eps, "calculated min: " << stats->GetMin() << " expected min: " << min);
    MITK_TEST_CONDITION(std::abs(stats->GetMax() - max) < mitk::eps, "calculated max: " << stats->GetMax() << " expected max: " << max);
    MITK_TEST_CONDITION(std::abs(stats->GetRMS() - RMS) < mitk::eps, "calculated RMS: " << stats->GetRMS() << " expected RMS: " << RMS);
    MITK_TEST_CONDITION(std::abs(stats->GetEntropy() - entropy) < mitk::eps, "calculated entropy: " << stats->GetEntropy() << " expected entropy: " << entropy);
    for (unsigned int i = 0; i < minIndex.size(); ++i)
    {
        MITK_TEST_CONDITION(std::abs(stats->GetMinIndex()[i] - minIndex[i]) < mitk::eps, "minIndex [" << i << "] = " << stats->GetMinIndex()[i] << " expected: " << minIndex[i]);
    }
    for (unsigned int i = 0; i < maxIndex.size(); ++i)
    {
        MITK_TEST_CONDITION(std::abs(stats->GetMaxIndex()[i] - maxIndex[i]) < mitk::eps, "maxIndex [" << i << "] = " << stats->GetMaxIndex()[i] << " expected: " << maxIndex[i]);
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
