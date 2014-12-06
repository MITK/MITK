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

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkImageStatisticsCalculator.h"
#include "mitkPlanarPolygon.h"

#include "mitkClassicDICOMSeriesReader.h"

#include "vtkStreamingDemandDrivenPipeline.h"

#include <mitkDataNodeFactory.h>
#include <mitkStandaloneDataStorage.h>


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
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp();

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

private:

  mitk::Image::Pointer m_Image;
  mitk::PlaneGeometry::Pointer m_Geometry;

  // calculate statistics for the given image and planarpolygon
  const mitk::ImageStatisticsCalculator::Statistics ComputeStatistics( mitk::Image::Pointer image,
                                                                       mitk::PlanarFigure::Pointer polygon );

  void VerifyStatistics(const mitk::ImageStatisticsCalculator::Statistics& stats,
                        double testMean, double testSD);
};

void mitkImageStatisticsCalculatorTestSuite::setUp()
{
  std::string filename = this->GetTestDataFilePath("ImageStatistics/testimage.dcm");
  if (filename.empty())
  {
    MITK_TEST_FAILED_MSG( << "Could not find test file" )
  }

  MITK_TEST_OUTPUT(<< "Loading test image '" << filename << "'")
  mitk::StringList files;
  files.push_back( filename );

  mitk::ClassicDICOMSeriesReader::Pointer reader = mitk::ClassicDICOMSeriesReader::New();
  reader->SetInputFiles( files );
  reader->AnalyzeInputFiles();
  reader->LoadImages();
  MITK_TEST_CONDITION_REQUIRED( reader->GetNumberOfOutputs() == 1, "Loaded one result from file" );

  m_Image = reader->GetOutput(0).GetMitkImage();
  MITK_TEST_CONDITION_REQUIRED( m_Image.IsNotNull(), "Loaded an mitk::Image" );

  m_Geometry = m_Image->GetSlicedGeometry()->GetPlaneGeometry(0);
  MITK_TEST_CONDITION_REQUIRED( m_Geometry.IsNotNull(), "Getting image geometry" )
}

void mitkImageStatisticsCalculatorTestSuite::TestCase1()
{
  /*****************************
   * one whole white pixel
   * -> mean of 255 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 255.0, 0.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase2()
{
  /*****************************
   * half pixel in x-direction (white)
   * -> mean of 255 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 255.0, 0.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase3()
{
  /*****************************
   * half pixel in diagonal-direction (white)
   * -> mean of 255 expected
   ******************************/
  mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
  figure1->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 10.5 ; pnt1[1] = 3.5;
  figure1->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
  figure1->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
  figure1->SetControlPoint( 2, pnt3, true );
  figure1->GetPolyLine(0);

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 255.0, 0.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase4()
{
  /*****************************
   * one pixel (white) + 2 half pixels (white) + 1 half pixel (black)
   * -> mean of 191.25 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 191.25, 127.5);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase5()
{
  /*****************************
   * whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 191.5 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 191.50, 89.80);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase6()
{
  /*****************************
   * quarter pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 191.5 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 191.5, 89.80);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase7()
{
  /*****************************
   * half pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
   * -> mean of 127.66 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure1.GetPointer()), 127.66, 127.5);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase8()
{
  /*****************************
   * whole pixel (gray)
   * -> mean of 128 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure2.GetPointer()), 128.0, 0.0);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase9()
{
  /*****************************
   * whole pixel (gray) + half pixel (white) in y-direction
   * -> mean of 191.5 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure2.GetPointer()), 191.5, 89.80);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase10()
{
  /*****************************
   * 2 whole pixel (white) + 2 whole pixel (black) in y-direction
   * -> mean of 127.66 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure2.GetPointer()), 127.66, 127.5);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase11()
{
  /*****************************
   * 9 whole pixels (white) + 3 half pixels (white)
   * + 3 whole pixel (black) [ + 3 slightly less than half pixels (black)]
   * -> mean of 204.0 expected
   ******************************/
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

  this->VerifyStatistics(ComputeStatistics(m_Image, figure2.GetPointer()), 204.0, 105.58 );
}

void mitkImageStatisticsCalculatorTestSuite::TestCase12()
{
  /*****************************
   * half pixel (white) + whole pixel (white) + half pixel (black)
   * -> mean of 212.66 expected
   ******************************/
  mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
  figure2->SetPlaneGeometry( m_Geometry );
  mitk::Point2D pnt1; pnt1[0] = 9.5; pnt1[1] = 0.5;
  figure2->PlaceFigure( pnt1 );

  mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 2.5;
  figure2->SetControlPoint( 1, pnt2, true );
  mitk::Point2D pnt3; pnt3[0] = 11.5; pnt3[1] = 2.5;
  figure2->SetControlPoint( 2, pnt3, true );
  figure2->GetPolyLine(0);

  this->VerifyStatistics(ComputeStatistics(m_Image, figure2.GetPointer()), 212.66, 73.32);
}

const mitk::ImageStatisticsCalculator::Statistics
mitkImageStatisticsCalculatorTestSuite::ComputeStatistics( mitk::Image::Pointer image, mitk::PlanarFigure::Pointer polygon )
{
  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
  statisticsCalculator->SetImage( image );
  statisticsCalculator->SetMaskingModeToPlanarFigure();
  statisticsCalculator->SetPlanarFigure( polygon );

  statisticsCalculator->ComputeStatistics();

  return statisticsCalculator->GetStatistics();
}

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(const mitk::ImageStatisticsCalculator::Statistics& stats,
                                                              double testMean, double testSD)
{
  int tmpMean = stats.GetMean() * 100;
  double calculatedMean = tmpMean / 100.0;
  MITK_TEST_CONDITION( calculatedMean == testMean,
                       "Calculated mean grayvalue '" << calculatedMean <<
                       "'  is equal to the desired value '" << testMean << "'" );

  int tmpSD = stats.GetSigma() * 100;
  double calculatedSD = tmpSD / 100.0;
  MITK_TEST_CONDITION( calculatedSD == testSD,
                       "Calculated grayvalue sd '" << calculatedSD <<
                       "'  is equal to the desired value '" << testSD <<"'" );
}

void mitkImageStatisticsCalculatorTestSuite::TestUninitializedImage()
{
  /*****************************
  * loading uninitialized image to datastorage
  ******************************/
  MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception)
  mitk::Image::Pointer image = mitk::Image::New();
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(image);

  mitk::ImageStatisticsCalculator::Pointer is = mitk::ImageStatisticsCalculator::New();
  is->ComputeStatistics();
  MITK_TEST_FOR_EXCEPTION_END(mitk::Exception)
}

MITK_TEST_SUITE_REGISTRATION(mitkImageStatisticsCalculator)
