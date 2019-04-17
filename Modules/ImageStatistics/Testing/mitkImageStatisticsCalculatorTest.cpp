
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

#include <mitkIOUtil.h>

#include <mitkPlanarFigureMaskGenerator.h>
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
  MITK_TEST(TestUS4DCroppedAllTimesteps);
  MITK_TEST(TestUS4DCropped3DMask);
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
  void TestUS4DCroppedAllTimesteps();
  void TestUS4DCropped3DMask();
private:
	mitk::Image::ConstPointer m_TestImage;

	mitk::Image::ConstPointer m_Pic3DCroppedImage;
	mitk::Image::Pointer m_Pic3DCroppedBinMask;
	mitk::Image::Pointer m_Pic3DCroppedMultilabelMask;
	mitk::PlanarFigure::Pointer m_Pic3DCroppedPlanarFigure;

	mitk::Image::ConstPointer m_US4DCroppedImage;
	mitk::Image::Pointer m_US4DCroppedBinMask;
	mitk::Image::Pointer m_US4DCroppedMultilabelMask;
	mitk::Image::Pointer m_US4DCropped3DBinMask;
	mitk::PlanarFigure::Pointer m_US4DCroppedPlanarFigure;

	mitk::PlaneGeometry::Pointer m_Geometry;

	// creates a polygon given a geometry and a vector of 2d points
	mitk::PlanarPolygon::Pointer GeneratePlanarPolygon(mitk::PlaneGeometry::Pointer geometry, std::vector <mitk::Point2D> points);

	// universal function to calculate statistics
	const mitk::ImageStatisticsContainer::Pointer ComputeStatistics(mitk::Image::ConstPointer image,
		mitk::MaskGenerator::Pointer maskGen = nullptr,
		mitk::MaskGenerator::Pointer secondardMaskGen = nullptr,
		unsigned short label = 1);

	void VerifyStatistics(mitk::ImageStatisticsContainer::ImageStatisticsObject stats,
		mitk::ImageStatisticsContainer::RealType testMean, mitk::ImageStatisticsContainer::RealType testSD, mitk::ImageStatisticsContainer::RealType testMedian = 0);

	// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
	void VerifyStatistics(mitk::ImageStatisticsContainer::ImageStatisticsObject stats,
		mitk::ImageStatisticsContainer::VoxelCountType N,
		mitk::ImageStatisticsContainer::RealType mean,
		mitk::ImageStatisticsContainer::RealType MPP,
		mitk::ImageStatisticsContainer::RealType skewness,
		mitk::ImageStatisticsContainer::RealType kurtosis,
		mitk::ImageStatisticsContainer::RealType variance,
		mitk::ImageStatisticsContainer::RealType stdev,
		mitk::ImageStatisticsContainer::RealType min,
		mitk::ImageStatisticsContainer::RealType max,
		mitk::ImageStatisticsContainer::RealType RMS,
		mitk::ImageStatisticsContainer::IndexType minIndex,
		mitk::ImageStatisticsContainer::IndexType maxIndex);
};

void mitkImageStatisticsCalculatorTestSuite::TestUninitializedImage()
{
	/*****************************
	* loading uninitialized image to datastorage
	******************************/
	MITK_INFO << std::endl << "Test uninitialized image: -----------------------------------------------------------------------------------";
	mitk::Image::Pointer image = mitk::Image::New();
	mitk::DataNode::Pointer node = mitk::DataNode::New();
	node->SetData(image);

	mitk::ImageStatisticsCalculator::Pointer is = mitk::ImageStatisticsCalculator::New();
	CPPUNIT_ASSERT_THROW(is->GetStatistics(), mitk::Exception);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase1()
{
	/*****************************
	 * one whole white pixel
	 * -> mean of 255 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 1:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::Point2D pnt1; pnt1[0] = 10.5; pnt1[1] = 3.5;
	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	mitk::Point2D pnt4; pnt4[0] = 10.5; pnt4[1] = 4.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);
	mitk::ImageStatisticsContainer::Pointer statisticsContainer;

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
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

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 10.0; pnt1[1] = 3.5;
	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	mitk::Point2D pnt4; pnt4[0] = 10.0; pnt4[1] = 4.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
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

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 10.5; pnt1[1] = 3.5;
	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
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

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 1.1; pnt1[1] = 1.1;
	mitk::Point2D pnt2; pnt2[0] = 2.0; pnt2[1] = 2.0;
	mitk::Point2D pnt3; pnt3[0] = 3.0; pnt3[1] = 1.0;
	mitk::Point2D pnt4; pnt4[0] = 2.0; pnt4[1] = 0.0;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 191.25, 110.41823898251593, 253.72499847412109);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase5()
{
	/*****************************
	 * whole pixel (white) + half pixel (gray) in x-direction
	 * -> mean of 191.5 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 5:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 191.50, 63.50, 128.63499999046327);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase6()
{
	/*****************************
	 * quarter pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
	 * -> mean of 191.5 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 6:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
	mitk::Point2D pnt2; pnt2[0] = 9.25; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 9.25; pnt3[1] = 4.5;
	mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 191.5, 63.50, 128.63499999046327);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase7()
{
	/*****************************
	 * half pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
	 * -> mean of 127.66 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 7:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
	mitk::Point2D pnt2; pnt2[0] = 9.0; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 9.0; pnt3[1] = 4.0;
	mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.0;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 127.666666666666667, 104.10358089689113, 128.7750015258789);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase8()
{
	/*****************************
	 * whole pixel (gray)
	 * -> mean of 128 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 8:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
	mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 11.5;
	mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 11.5;
	mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
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

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
	mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 12.0;
	mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 12.0;
	mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 191.5, 63.50, 128.63499999046327);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase10()
{
	/*****************************
	 * 2 whole pixel (white) + 2 whole pixel (black) in y-direction
	 * -> mean of 127.66 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 10:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
	mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 13.5;
	mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 13.5;
	mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 127.666666666666667, 104.10358089689113, 128.7750015258789);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase11()
{
	/*****************************
	 * 9 whole pixels (white) + 3 half pixels (white)
	 * + 3 whole pixel (black) [ + 3 slightly less than half pixels (black)]
	 * -> mean of 204.0 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 11:-----------------------------------------------------------------------------------";
	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 0.5; pnt1[1] = 0.5;
	mitk::Point2D pnt2; pnt2[0] = 3.5; pnt2[1] = 3.5;
	mitk::Point2D pnt3; pnt3[0] = 8.4999; pnt3[1] = 3.5;
	mitk::Point2D pnt4; pnt4[0] = 5.4999; pnt4[1] = 0.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3,pnt4 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 204.0, 102.00, 253.724998474121083);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase12()
{
	/*****************************
	 * half pixel (white) + whole pixel (white) + half pixel (black)
	 * -> mean of 212.66 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 12:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.nrrd");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);

	mitk::Point2D pnt1; pnt1[0] = 9.5; pnt1[1] = 0.5;
	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 2.5;
	mitk::Point2D pnt3; pnt3[0] = 11.5; pnt3[1] = 2.5;
	std::vector<mitk::Point2D> points{ pnt1,pnt2,pnt3 };
        auto figure = GeneratePlanarPolygon(m_Geometry, points);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(m_TestImage);
        planFigMaskGen->SetPlanarFigure(figure.GetPointer());

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_TestImage, planFigMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 212.666666666666667, 59.8683741404609923, 254.36499786376954);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedNoMask()
{
	MITK_INFO << std::endl << "Test Pic3D cropped without mask:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 27;
	mitk::ImageStatisticsContainer::RealType expected_mean = -564.1481481481481481;
	mitk::ImageStatisticsContainer::RealType expected_MPP = 113.66666666666667;
        //mitk::ImageStatisticsContainer::RealType expected_median = -825;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0.7120461106763573;
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1.8794464383714844;
	mitk::ImageStatisticsContainer::RealType expected_variance = 140541.38545953357;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 374.88849736892911;
	mitk::ImageStatisticsContainer::RealType expected_min = -927;
	mitk::ImageStatisticsContainer::RealType expected_max = 147;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 677.35110431630551;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 2;
	expected_minIndex[1] = 1;
	expected_minIndex[2] = 1;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 2;

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_Pic3DCroppedImage));
	auto statisticsObject = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObject,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedBinMask()
{
	MITK_INFO << std::endl << "Test Pic3D cropped binary mask:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	std::string Pic3DCroppedBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedBinMask.nrrd");
	m_Pic3DCroppedBinMask = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedBinMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D binary mask", m_Pic3DCroppedBinMask.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1.0765697398089618;
	mitk::ImageStatisticsContainer::RealType expected_MPP = -nan("");
	mitk::ImageStatisticsContainer::RealType expected_max = -22;
	mitk::ImageStatisticsContainer::RealType expected_mean = -464;
	mitk::ImageStatisticsContainer::RealType expected_min = -846;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 4;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 595.42631785973322;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0.0544059290851858;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 373.14407405183323;
	mitk::ImageStatisticsContainer::RealType expected_variance = 139236.50;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 1;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 0;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
	imgMaskGen->SetImageMask(m_Pic3DCroppedBinMask);
	imgMaskGen->SetInputImage(m_Pic3DCroppedImage);
	imgMaskGen->SetTimeStep(0);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_Pic3DCroppedImage, imgMaskGen.GetPointer(), nullptr, 1));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObjectTimestep0,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedMultilabelMask()
{
	MITK_INFO << std::endl << "Test Pic3D cropped multilabel mask:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	std::string Pic3DCroppedMultilabelMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedMultilabelMask.nrrd");
	m_Pic3DCroppedMultilabelMask = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedMultilabelMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D multilabel mask", m_Pic3DCroppedMultilabelMask.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1.5;
	mitk::ImageStatisticsContainer::RealType expected_MPP = -nan("");
	mitk::ImageStatisticsContainer::RealType expected_max = -22;
	mitk::ImageStatisticsContainer::RealType expected_mean = -586.33333333333333;
	mitk::ImageStatisticsContainer::RealType expected_min = -916;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 3;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 710.3006405741163;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0.6774469597523700;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 400.92421007245525;
	mitk::ImageStatisticsContainer::RealType expected_variance = 160740.22222222222;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 2;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 1;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
	imgMaskGen->SetImageMask(m_Pic3DCroppedMultilabelMask);
	imgMaskGen->SetInputImage(m_Pic3DCroppedImage);
	imgMaskGen->SetTimeStep(0);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_Pic3DCroppedImage, imgMaskGen.GetPointer(), nullptr, 2));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObjectTimestep0,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedPlanarFigure()
{
	MITK_INFO << std::endl << "Test Pic3D cropped planar figure:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	std::string Pic3DCroppedPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedPF.pf");
	m_Pic3DCroppedPlanarFigure = mitk::IOUtil::Load<mitk::PlanarFigure>(Pic3DCroppedPlanarFigureFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D planar figure", m_Pic3DCroppedPlanarFigure.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1;
	mitk::ImageStatisticsContainer::RealType expected_MPP = -nan("");
	mitk::ImageStatisticsContainer::RealType expected_max = -67;
	mitk::ImageStatisticsContainer::RealType expected_mean = -446;
	mitk::ImageStatisticsContainer::RealType expected_min = -825;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 2;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 585.28369189650243;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 379;
	mitk::ImageStatisticsContainer::RealType expected_variance = 143641;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 1;
	expected_minIndex[1] = 1;
	expected_minIndex[2] = 1;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 1;

	mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
	pfMaskGen->SetInputImage(m_Pic3DCroppedImage);
	pfMaskGen->SetPlanarFigure(m_Pic3DCroppedPlanarFigure);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_Pic3DCroppedImage, pfMaskGen.GetPointer()));
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObjectTimestep0,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedNoMaskTimeStep1()
{
	MITK_INFO << std::endl << "Test US4D cropped without mask timestep 1:-----------------------------------------------------------------------------------";

        std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1.5398359155908228;
	mitk::ImageStatisticsContainer::RealType expected_MPP = 157.74074074074073;
	mitk::ImageStatisticsContainer::RealType expected_max = 199;
	mitk::ImageStatisticsContainer::RealType expected_mean = 157.74074074074073;
	mitk::ImageStatisticsContainer::RealType expected_min = 101;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 27;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 160.991718213494823;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0.0347280313508018;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 32.189936997387058;
	mitk::ImageStatisticsContainer::RealType expected_variance = 1036.19204389574722;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 0;
	expected_minIndex[1] = 2;
	expected_minIndex[2] = 0;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageStatisticsContainer::Pointer statisticsContainer=mitk::ImageStatisticsContainer::New();
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_US4DCroppedImage));
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedBinMaskTimeStep1()
{
	MITK_INFO << std::endl << "Test US4D cropped with binary mask timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCroppedBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_croppedBinMask.nrrd");
	m_US4DCroppedBinMask = mitk::IOUtil::Load<mitk::Image>(US4DCroppedBinMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D binary mask", m_US4DCroppedBinMask.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1.5863739712889191;
	mitk::ImageStatisticsContainer::RealType expected_MPP = 166.75;
	mitk::ImageStatisticsContainer::RealType expected_max = 199;
	mitk::ImageStatisticsContainer::RealType expected_mean = 166.75;
	mitk::ImageStatisticsContainer::RealType expected_min = 120;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 4;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 169.70636405273669;
	mitk::ImageStatisticsContainer::RealType expected_skewness = -0.4285540263894276;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 31.538666744172936;
	mitk::ImageStatisticsContainer::RealType expected_variance = 994.6874999999999;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 0;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 2;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 1;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
	imgMask1->SetInputImage(m_US4DCroppedImage);
	imgMask1->SetImageMask(m_US4DCroppedBinMask);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer=mitk::ImageStatisticsContainer::New();
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_US4DCroppedImage, imgMask1.GetPointer(), nullptr, 1));
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedMultilabelMaskTimeStep1()
{
	MITK_INFO << std::endl << "Test US4D cropped with mulitlabel mask timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCroppedMultilabelMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_croppedMultilabelMask.nrrd");
	m_US4DCroppedMultilabelMask = mitk::IOUtil::Load<mitk::Image>(US4DCroppedMultilabelMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D multilabel mask", m_US4DCroppedMultilabelMask.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1.0432484564918287;
	mitk::ImageStatisticsContainer::RealType expected_MPP = 159.75;
	mitk::ImageStatisticsContainer::RealType expected_max = 199;
	mitk::ImageStatisticsContainer::RealType expected_mean = 159.75;
	mitk::ImageStatisticsContainer::RealType expected_min = 120;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 4;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 163.74446555532802;
	mitk::ImageStatisticsContainer::RealType expected_skewness = -0.004329226115093;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 35.947009611371016;
	mitk::ImageStatisticsContainer::RealType expected_variance = 1292.187500000000227;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 0;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 2;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
	imgMask1->SetInputImage(m_US4DCroppedImage);
	imgMask1->SetImageMask(m_US4DCroppedMultilabelMask);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_US4DCroppedImage, imgMask1.GetPointer(), nullptr, 1));
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedPlanarFigureTimeStep1()
{
	MITK_INFO << std::endl << "Test US4D cropped planar figure timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCroppedPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_croppedPF.pf");
	m_US4DCroppedPlanarFigure = mitk::IOUtil::Load<mitk::PlanarFigure>(US4DCroppedPlanarFigureFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D planar figure", m_US4DCroppedPlanarFigure.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1;
	mitk::ImageStatisticsContainer::RealType expected_MPP = 172.5;
	mitk::ImageStatisticsContainer::RealType expected_max = 197;
	mitk::ImageStatisticsContainer::RealType expected_mean = 172.5;
	mitk::ImageStatisticsContainer::RealType expected_min = 148;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 2;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 174.23116827938679;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 24.5;
	mitk::ImageStatisticsContainer::RealType expected_variance = 600.25;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 2;
	expected_minIndex[1] = 2;
	expected_minIndex[2] = 2;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 2;
	expected_maxIndex[1] = 2;
	expected_maxIndex[2] = 1;

	mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
	pfMaskGen->SetInputImage(m_US4DCroppedImage);
	pfMaskGen->SetPlanarFigure(m_US4DCroppedPlanarFigure);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer;
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_US4DCroppedImage, pfMaskGen.GetPointer()));
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedAllTimesteps()
{
	MITK_INFO << std::endl << "Test US4D cropped all timesteps:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());
	mitk::ImageStatisticsContainer::Pointer statisticsContainer=mitk::ImageStatisticsContainer::New();
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_US4DCroppedImage));
	for (int i = 0; i < 4; i++)
	{
		CPPUNIT_ASSERT_MESSAGE("Error computing statistics for multiple timestep", statisticsContainer->TimeStepExists(i));
	}
}

void mitkImageStatisticsCalculatorTestSuite::TestUS4DCropped3DMask()
{
	MITK_INFO << std::endl << "Test US4D cropped with 3D binary Mask:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCropped3DBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped3DBinMask.nrrd");
	m_US4DCropped3DBinMask = mitk::IOUtil::Load<mitk::Image>(US4DCropped3DBinMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D binary mask", m_US4DCropped3DBinMask.IsNotNull());
	//calculated ground truth via script
	mitk::ImageStatisticsContainer::RealType expected_kurtosis = 1;
	mitk::ImageStatisticsContainer::RealType expected_MPP = 198;
	mitk::ImageStatisticsContainer::RealType expected_max = 199;
	mitk::ImageStatisticsContainer::RealType expected_mean = 198;
	mitk::ImageStatisticsContainer::RealType expected_min = 197;
	mitk::ImageStatisticsContainer::VoxelCountType expected_N = 2;
	mitk::ImageStatisticsContainer::RealType expected_RMS = 198.00252523642217;
	mitk::ImageStatisticsContainer::RealType expected_skewness = 0;
	mitk::ImageStatisticsContainer::RealType expected_standarddev = 1;
	mitk::ImageStatisticsContainer::RealType expected_variance = 1;
	mitk::ImageStatisticsContainer::IndexType expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 1;
	expected_minIndex[1] = 2;
	expected_minIndex[2] = 1;

	mitk::ImageStatisticsContainer::IndexType expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 1;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
	imgMask1->SetInputImage(m_US4DCroppedImage);
	imgMask1->SetImageMask(m_US4DCropped3DBinMask);

	mitk::ImageStatisticsContainer::Pointer statisticsContainer = mitk::ImageStatisticsContainer::New();
	CPPUNIT_ASSERT_NO_THROW(statisticsContainer = ComputeStatistics(m_US4DCroppedImage, imgMask1.GetPointer(), nullptr, 1));
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		expected_skewness,
		expected_kurtosis,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		expected_minIndex,
		expected_maxIndex);
}

mitk::PlanarPolygon::Pointer mitkImageStatisticsCalculatorTestSuite::GeneratePlanarPolygon(mitk::PlaneGeometry::Pointer geometry, std::vector <mitk::Point2D> points)
{
	mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
	figure->SetPlaneGeometry(geometry);
	figure->PlaceFigure(points[0]);
        for (unsigned int i = 1; i < points.size(); i++)
	{
		figure->SetControlPoint(i, points[i], true);
	}
	return figure;
}

const mitk::ImageStatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatistics(mitk::Image::ConstPointer image,
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

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsContainer::ImageStatisticsObject stats,
	mitk::ImageStatisticsContainer::RealType testMean, mitk::ImageStatisticsContainer::RealType testSD, mitk::ImageStatisticsContainer::RealType testMedian)
{
    mitk::ImageStatisticsContainer::RealType meanObject = 0;
    mitk::ImageStatisticsContainer::RealType standardDeviationObject = 0;
    mitk::ImageStatisticsContainer::RealType medianObject = 0;
	CPPUNIT_ASSERT_NO_THROW(meanObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEAN()));
	CPPUNIT_ASSERT_NO_THROW(standardDeviationObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::STANDARDDEVIATION()));
	CPPUNIT_ASSERT_NO_THROW(medianObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEDIAN()));
	CPPUNIT_ASSERT_MESSAGE("Calculated mean grayvalue is not equal to the desired value.", std::abs(meanObject - testMean) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated grayvalue sd is not equal to the desired value.", std::abs(standardDeviationObject - testSD) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated median grayvalue is not equal to the desired value.", std::abs(medianObject - testMedian) < mitk::eps);
}

// T26098 histogram statistics need to be tested (median, uniformity, UPP, entropy)
void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsContainer::ImageStatisticsObject stats,
	mitk::ImageStatisticsContainer::VoxelCountType N,
	mitk::ImageStatisticsContainer::RealType mean,
	mitk::ImageStatisticsContainer::RealType MPP,
	mitk::ImageStatisticsContainer::RealType skewness,
	mitk::ImageStatisticsContainer::RealType kurtosis,
	mitk::ImageStatisticsContainer::RealType variance,
	mitk::ImageStatisticsContainer::RealType stdev,
	mitk::ImageStatisticsContainer::RealType min,
	mitk::ImageStatisticsContainer::RealType max,
	mitk::ImageStatisticsContainer::RealType RMS,
	mitk::ImageStatisticsContainer::IndexType minIndex,
	mitk::ImageStatisticsContainer::IndexType maxIndex)
{
  mitk::ImageStatisticsContainer::VoxelCountType numberOfVoxelsObject;
  mitk::ImageStatisticsContainer::RealType meanObject = 0;
  mitk::ImageStatisticsContainer::RealType mppObject = 0;
  mitk::ImageStatisticsContainer::RealType skewnessObject = 0;
  mitk::ImageStatisticsContainer::RealType kurtosisObject = 0;
  mitk::ImageStatisticsContainer::RealType varianceObject = 0;
  mitk::ImageStatisticsContainer::RealType standardDeviationObject = 0;
  mitk::ImageStatisticsContainer::RealType minObject = 0;
  mitk::ImageStatisticsContainer::RealType maxObject = 0;
  mitk::ImageStatisticsContainer::RealType rmsObject = 0;
  mitk::ImageStatisticsContainer::IndexType minIndexObject = {0, 0, 0};
  mitk::ImageStatisticsContainer::IndexType maxIndexObject = {0, 0, 0};
	CPPUNIT_ASSERT_NO_THROW(numberOfVoxelsObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::VoxelCountType>(mitk::ImageStatisticsConstants::NUMBEROFVOXELS()));
	CPPUNIT_ASSERT_NO_THROW(meanObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEAN()));
	CPPUNIT_ASSERT_NO_THROW(mppObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MPP()));
	CPPUNIT_ASSERT_NO_THROW(skewnessObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::SKEWNESS()));
	CPPUNIT_ASSERT_NO_THROW(kurtosisObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::KURTOSIS()));
	CPPUNIT_ASSERT_NO_THROW(varianceObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::VARIANCE()));
	CPPUNIT_ASSERT_NO_THROW(standardDeviationObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::STANDARDDEVIATION()));
	CPPUNIT_ASSERT_NO_THROW(minObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MINIMUM()));
	CPPUNIT_ASSERT_NO_THROW(maxObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MAXIMUM()));
	CPPUNIT_ASSERT_NO_THROW(rmsObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::RMS()));
	CPPUNIT_ASSERT_NO_THROW(minIndexObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::IndexType>(mitk::ImageStatisticsConstants::MINIMUMPOSITION()));
	CPPUNIT_ASSERT_NO_THROW(maxIndexObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::IndexType>(mitk::ImageStatisticsConstants::MAXIMUMPOSITION()));

	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", numberOfVoxelsObject - N == 0);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(meanObject - mean) < mitk::eps);
	// in three test cases MPP is None because the roi has no positive pixels
	if (!std::isnan(mppObject))
	{
		CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(mppObject - MPP) < mitk::eps);
	}
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(skewnessObject - skewness) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(kurtosisObject - kurtosis) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(varianceObject - variance) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(standardDeviationObject - stdev) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(minObject - min) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(maxObject - max) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(rmsObject - RMS) < mitk::eps);
	for (unsigned int i = 0; i < minIndex.size(); ++i)
	{
		CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(minIndexObject[i] - minIndex[i]) < mitk::eps);
	}
	for (unsigned int i = 0; i < maxIndex.size(); ++i)
	{
		CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(maxIndexObject[i] - maxIndex[i]) < mitk::eps);
	}
}
MITK_TEST_SUITE_REGISTRATION(mitkImageStatisticsCalculator)
