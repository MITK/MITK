
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
	MITK_TEST(TestUS4DCroppedBinMask);
	MITK_TEST(TestUS4DCroppedMultilabelMask);
	MITK_TEST(TestUS4DCroppedPlanarFigure);
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
	void TestUS4DCroppedBinMask();
	void TestUS4DCroppedMultilabelMask();
	void TestUS4DCroppedPlanarFigure();

private:

	mitk::Image::ConstPointer m_TestImage;

	mitk::Image::ConstPointer m_Pic3DCroppedImage;
	mitk::Image::Pointer m_Pic3DCroppedBinMask;
	mitk::Image::Pointer m_Pic3DCroppedMultilabelMask;
	mitk::PlanarFigure::Pointer m_Pic3DCroppedPlanarFigure;

	mitk::Image::ConstPointer m_US4DCroppedImage;
	mitk::Image::Pointer m_US4DCroppedBinMask;
	mitk::Image::Pointer m_US4DCroppedMultilabelMask;
	mitk::PlanarFigure::Pointer m_US4DCroppedPlanarFigure;

	mitk::PlaneGeometry::Pointer m_Geometry;
	// calculate statistics for the given image and planarpolygon
	const mitk::ImageStatisticsContainer::Pointer ComputeStatistics(mitk::Image::ConstPointer image,
		mitk::PlanarFigure::Pointer polygon);

	// calculate statistics for the given image and mask
	const mitk::ImageStatisticsContainer::Pointer ComputeStatistics(mitk::Image::ConstPointer image,
		mitk::Image::Pointer image_mask);

	// universal function to calculate statistics
	const mitk::ImageStatisticsContainer::Pointer ComputeStatisticsNew(mitk::Image::ConstPointer image,
		mitk::MaskGenerator::Pointer maskGen = nullptr,
		mitk::MaskGenerator::Pointer secondardMaskGen = nullptr,
		unsigned short label = 1);

	void VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
		double testMean, double testSD, double testMedian = 0);

	void VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
		unsigned long N,
		double mean,
		double MPP,
		//double median,
		double skewness,
		double kurtosis,
		//double uniformity,
		//double UPP,
		double variance,
		double stdev,
		double min,
		double max,
		double RMS,
		//double entropy,
		vnl_vector<int> minIndex,
		vnl_vector<int> maxIndex);
};

void mitkImageStatisticsCalculatorTestSuite::TestUninitializedImage()
{
	/*****************************
	* loading uninitialized image to datastorage
	******************************/
	MITK_INFO << std::endl << "Test uninitialized image: -----------------------------------------------------------------------------------";
	CPPUNIT_ASSERT_THROW(
		mitk::Image::Pointer image = mitk::Image::New();
	mitk::DataNode::Pointer node = mitk::DataNode::New();
	node->SetData(image);

	mitk::ImageStatisticsCalculator::Pointer is = mitk::ImageStatisticsCalculator::New();
	is->GetStatistics(), mitk::Exception);
}

void mitkImageStatisticsCalculatorTestSuite::TestCase1()
{
	/*****************************
	 * one whole white pixel
	 * -> mean of 255 expected
	 ******************************/
	MITK_INFO << std::endl << "Test case 1:-----------------------------------------------------------------------------------";

	std::string filename = this->GetTestDataFilePath("ImageStatisticsTestData/testimage.dcm");
	m_TestImage = mitk::IOUtil::Load<mitk::Image>(filename);
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 10.5; pnt1[1] = 3.5;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	figure1->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 10.5; pnt4[1] = 4.5;
	figure1->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 10.0; pnt1[1] = 3.5;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	figure1->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 10.0; pnt4[1] = 4.5;
	figure1->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 10.5; pnt1[1] = 3.5;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	figure1->SetControlPoint(2, pnt3, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 1.1; pnt1[1] = 1.1;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 2.0; pnt2[1] = 2.0;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 3.0; pnt3[1] = 1.0;
	figure1->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 2.0; pnt4[1] = 0.0;
	figure1->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
	figure1->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
	figure1->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.25; pnt2[1] = 3.5;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 9.25; pnt3[1] = 4.5;
	figure1->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
	figure1->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
	figure1->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
	figure1->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.0; pnt2[1] = 3.5;
	figure1->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 9.0; pnt3[1] = 4.0;
	figure1->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.0;
	figure1->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
	figure2->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
	figure2->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 11.5;
	figure2->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 11.5;
	figure2->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
	figure2->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
	figure2->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
	figure2->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 12.0;
	figure2->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 12.0;
	figure2->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
	figure2->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
	figure2->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
	figure2->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 13.5;
	figure2->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 13.5;
	figure2->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
	figure2->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
	figure2->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 0.5; pnt1[1] = 0.5;
	figure2->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 3.5; pnt2[1] = 3.5;
	figure2->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 8.4999; pnt3[1] = 3.5;
	figure2->SetControlPoint(2, pnt3, true);
	mitk::Point2D pnt4; pnt4[0] = 5.4999; pnt4[1] = 0.5;
	figure2->SetControlPoint(3, pnt4, true);
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
	CPPUNIT_ASSERT_MESSAGE("Failed loading an mitk::Image", m_TestImage.IsNotNull());

	m_Geometry = m_TestImage->GetSlicedGeometry()->GetPlaneGeometry(0);
	CPPUNIT_ASSERT_MESSAGE("Failed getting image geometry", m_Geometry.IsNotNull());

	mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
	figure2->SetPlaneGeometry(m_Geometry);
	mitk::Point2D pnt1; pnt1[0] = 9.5; pnt1[1] = 0.5;
	figure2->PlaceFigure(pnt1);

	mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 2.5;
	figure2->SetControlPoint(1, pnt2, true);
	mitk::Point2D pnt3; pnt3[0] = 11.5; pnt3[1] = 2.5;
	figure2->SetControlPoint(2, pnt3, true);
	figure2->GetPolyLine(0);

	auto statisticsContainer = ComputeStatistics(m_TestImage, figure2.GetPointer());
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	this->VerifyStatistics(statisticsObjectTimestep0, 212.66, 59.860, 248.640);
}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedNoMask()
{
	MITK_INFO << std::endl << "Test Pic3D cropped without mask:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	unsigned long expected_N = 27;
	double expected_mean = -564.1481481481481481;
	double expected_MPP = 113.66666666666667;
	double expected_median = -825;
	double expected_skewness = 0.7120461106763573;
	double expected_kurtosis = 1.8794464383714844;
	//double expected_uniformity = ;
	//double expected_UPP = ;
	double expected_variance = 140541.38545953357;
	double expected_standarddev = 374.88849736892911;
	double expected_min = -927;
	double expected_max = 147;
	double expected_RMS = 677.35110431630551;
	//double expected_entropy = ;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 2;
	expected_minIndex[1] = 1;
	expected_minIndex[2] = 1;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 2;

	auto statisticsContainer = ComputeStatisticsNew(m_Pic3DCroppedImage);
	auto statisticsObject = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObject,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedBinMask()
{
	MITK_INFO << std::endl << "Test Pic3D cropped binary mask:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	std::string Pic3DCroppedBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedBinMask.nrrd");
	m_Pic3DCroppedBinMask = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedBinMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D binary mask", m_Pic3DCroppedBinMask.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1.0765697398089618;
	double expected_MPP = -nan("");
	double expected_max = -22;
	double expected_mean = -464;
	//double expected_median = -494;
	double expected_min = -846;
	unsigned long expected_N = 4;
	double expected_RMS = 595.42631785973322;
	double expected_skewness = 0.0544059290851858;
	double expected_standarddev = 373.14407405183323;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 139236.50;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 1;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 0;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
	imgMaskGen->SetImageMask(m_Pic3DCroppedBinMask);
	imgMaskGen->SetInputImage(m_Pic3DCroppedImage);
	imgMaskGen->SetTimeStep(0);

	auto statisticsContainer = ComputeStatisticsNew(m_Pic3DCroppedImage, imgMaskGen.GetPointer(), nullptr, 1);
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObjectTimestep0,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}
void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedMultilabelMask()
{
	MITK_INFO << std::endl << "Test Pic3D cropped multilabel mask:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	std::string Pic3DCroppedMultilabelMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedMultilabelMask.nrrd");
	m_Pic3DCroppedMultilabelMask = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedMultilabelMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D multilabel mask", m_Pic3DCroppedMultilabelMask.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1.5;
	double expected_MPP = -nan("");
	double expected_max = -22;
	double expected_mean = -586.33333333333333;
	//double expected_median = ;
	double expected_min = -916;
	unsigned long expected_N = 3;
	double expected_RMS = 710.3006405741163;
	double expected_skewness = 0.6774469597523700;
	double expected_standarddev = 400.92421007245525;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 160740.22222222222;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 2;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 1;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
	imgMaskGen->SetImageMask(m_Pic3DCroppedMultilabelMask);
	imgMaskGen->SetInputImage(m_Pic3DCroppedImage);
	imgMaskGen->SetTimeStep(0);

	auto statisticsContainer = ComputeStatisticsNew(m_Pic3DCroppedImage, imgMaskGen.GetPointer(), nullptr, 2);
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObjectTimestep0,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}
void mitkImageStatisticsCalculatorTestSuite::TestPic3DCroppedPlanarFigure()
{
	MITK_INFO << std::endl << "Test Pic3D cropped planar figure:-----------------------------------------------------------------------------------";

	std::string Pic3DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_cropped.nrrd");
	m_Pic3DCroppedImage = mitk::IOUtil::Load<mitk::Image>(Pic3DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D_cropped", m_Pic3DCroppedImage.IsNotNull());

	std::string Pic3DCroppedPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/Pic3D_croppedPF.pf");
	m_Pic3DCroppedPlanarFigure = mitk::IOUtil::Load<mitk::PlanarFigure>(Pic3DCroppedPlanarFigureFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading Pic3D planar figure", m_Pic3DCroppedPlanarFigure.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1;
	double expected_MPP = -nan("");
	double expected_max = -67;
	double expected_mean = -446;
	//double expected_median = -446;
	double expected_min = -825;
	unsigned long expected_N = 2;
	double expected_RMS = 585.28369189650243;
	double expected_skewness = 0;
	double expected_standarddev = 379;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 143641;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 1;
	expected_minIndex[1] = 1;
	expected_minIndex[2] = 1;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 1;

	mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
	pfMaskGen->SetInputImage(m_Pic3DCroppedImage);
	pfMaskGen->SetPlanarFigure(m_Pic3DCroppedPlanarFigure);

	auto statisticsContainer = ComputeStatisticsNew(m_Pic3DCroppedImage, pfMaskGen.GetPointer());
	auto statisticsObjectTimestep0 = statisticsContainer->GetStatisticsForTimeStep(0);

	VerifyStatistics(statisticsObjectTimestep0,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}

void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedNoMaskTimeStep1()
{
	MITK_INFO << std::endl << "Test US4D cropped without mask timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_Cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1.5398359155908228;
	double expected_MPP = 157.74074074074073;
	double expected_max = 199;
	double expected_mean = 157.74074074074073;
	//double expected_median = 156;
	double expected_min = 101;
	unsigned long expected_N = 27;
	double expected_RMS = 160.991718213494823;
	double expected_skewness = 0.0347280313508018;
	double expected_standarddev = 32.189936997387058;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 1036.19204389574722;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 0;
	expected_minIndex[1] = 2;
	expected_minIndex[2] = 0;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	auto statisticsContainer = ComputeStatisticsNew(m_US4DCroppedImage);
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedBinMask()
{
	MITK_INFO << std::endl << "Test US4D cropped with binary mask timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCroppedBinMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_croppedBinMask.nrrd");
	m_US4DCroppedBinMask = mitk::IOUtil::Load<mitk::Image>(US4DCroppedBinMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D binary mask", m_US4DCroppedBinMask.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1.5863739712889191;
	double expected_MPP = 166.75;
	double expected_max = 199;
	double expected_mean = 166.75;
	//double expected_median = 174;
	double expected_min = 120;
	unsigned long expected_N = 4;
	double expected_RMS = 169.70636405273669;
	double expected_skewness = -0.4285540263894276;
	double expected_standarddev = 31.538666744172936;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 994.6874999999999;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 0;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 2;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 1;
	expected_maxIndex[1] = 1;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
	imgMask1->SetInputImage(m_US4DCroppedImage);
	imgMask1->SetImageMask(m_US4DCroppedBinMask);

	auto statisticsContainer = ComputeStatisticsNew(m_US4DCroppedImage, imgMask1.GetPointer(), nullptr, 1);
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedMultilabelMask()
{
	MITK_INFO << std::endl << "Test US4D cropped with mulitlabel mask timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCroppedMultilabelMaskFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_croppedMultilabelMask.nrrd");
	m_US4DCroppedMultilabelMask = mitk::IOUtil::Load<mitk::Image>(US4DCroppedMultilabelMaskFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D multilabel mask", m_US4DCroppedMultilabelMask.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1.0432484564918287;
	double expected_MPP = 159.75;
	double expected_max = 199;
	double expected_mean = 159.75;
	//double expected_median = ;
	double expected_min = 120;
	unsigned long expected_N = 4;
	double expected_RMS = 163.74446555532802;
	double expected_skewness = -0.004329226115093;
	double expected_standarddev = 35.947009611371016;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 1292.187500000000227;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 0;
	expected_minIndex[1] = 0;
	expected_minIndex[2] = 2;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 0;
	expected_maxIndex[1] = 0;
	expected_maxIndex[2] = 1;

	mitk::ImageMaskGenerator::Pointer imgMask1 = mitk::ImageMaskGenerator::New();
	imgMask1->SetInputImage(m_US4DCroppedImage);
	imgMask1->SetImageMask(m_US4DCroppedMultilabelMask);

	auto statisticsContainer = ComputeStatisticsNew(m_US4DCroppedImage, imgMask1.GetPointer(), nullptr, 1);
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}
void mitkImageStatisticsCalculatorTestSuite::TestUS4DCroppedPlanarFigure()
{
	MITK_INFO << std::endl << "Test US4D cropped planar figure timestep 1:-----------------------------------------------------------------------------------";

	std::string US4DCroppedFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_cropped.nrrd");
	m_US4DCroppedImage = mitk::IOUtil::Load<mitk::Image>(US4DCroppedFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D_cropped", m_US4DCroppedImage.IsNotNull());

	std::string US4DCroppedPlanarFigureFile = this->GetTestDataFilePath("ImageStatisticsTestData/US4D_croppedPF.pf");
	m_US4DCroppedPlanarFigure = mitk::IOUtil::Load<mitk::PlanarFigure>(US4DCroppedPlanarFigureFile);
	CPPUNIT_ASSERT_MESSAGE("Failed loading US4D planar figure", m_US4DCroppedPlanarFigure.IsNotNull());

	//double expected_entropy = ;
	double expected_kurtosis = 1;
	double expected_MPP = 172.5;
	double expected_max = 197;
	double expected_mean = 172.5;
	//double expected_median = ;
	double expected_min = 148;
	unsigned long expected_N = 2;
	double expected_RMS = 174.23116827938679;
	double expected_skewness = 0;
	double expected_standarddev = 24.5;
	//double expected_UPP = ;
	//double expected_uniformity = ;
	double expected_variance = 600.25;
	vnl_vector<int> expected_minIndex;
	expected_minIndex.set_size(3);
	expected_minIndex[0] = 2;
	expected_minIndex[1] = 2;
	expected_minIndex[2] = 2;

	vnl_vector<int> expected_maxIndex;
	expected_maxIndex.set_size(3);
	expected_maxIndex[0] = 2;
	expected_maxIndex[1] = 2;
	expected_maxIndex[2] = 1;

	mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
	pfMaskGen->SetInputImage(m_US4DCroppedImage);
	pfMaskGen->SetPlanarFigure(m_US4DCroppedPlanarFigure);

	auto statisticsContainer = ComputeStatisticsNew(m_US4DCroppedImage, pfMaskGen.GetPointer());
	auto statisticsObjectTimestep1 = statisticsContainer->GetStatisticsForTimeStep(1);

	VerifyStatistics(statisticsObjectTimestep1,
		expected_N,
		expected_mean,
		expected_MPP,
		//expected_median,
		expected_skewness,
		expected_kurtosis,
		//expected_uniformity,
		//expected_UPP,
		expected_variance,
		expected_standarddev,
		expected_min,
		expected_max,
		expected_RMS,
		//expected_entropy,
		expected_minIndex,
		expected_maxIndex);
}

const mitk::ImageStatisticsContainer::Pointer mitkImageStatisticsCalculatorTestSuite::ComputeStatistics(mitk::Image::ConstPointer image, mitk::PlanarFigure::Pointer polygon)
{
	mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
	statisticsCalculator->SetInputImage(image);

	statisticsCalculator->SetNBinsForHistogramStatistics(10);

	mitk::PlanarFigureMaskGenerator::Pointer planFigMaskGen = mitk::PlanarFigureMaskGenerator::New();
	planFigMaskGen->SetInputImage(image);
	planFigMaskGen->SetPlanarFigure(polygon);

	statisticsCalculator->SetMask(planFigMaskGen.GetPointer());

	try
	{
		return statisticsCalculator->GetStatistics();
	}
	catch (...)
	{
		return nullptr;
	}
}

const mitk::ImageStatisticsContainer::Pointer
mitkImageStatisticsCalculatorTestSuite::ComputeStatistics(mitk::Image::ConstPointer image, mitk::Image::Pointer image_mask)
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
	CPPUNIT_ASSERT_MESSAGE("Calculated mean grayvalue is not equal to the desired value.", calculatedMean == testMean);

	auto standardDeviation = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());
	int tmpSD = standardDeviation * 100;
	double calculatedSD = tmpSD / 100.0;
	CPPUNIT_ASSERT_MESSAGE("Calculated grayvalue sd is not equal to the desired value.", calculatedSD == testSD);

	auto median = stats.GetValueConverted<double>(mitk::ImageStatisticsConstants::MEDIAN());
	int tmpMedian = median * 100;
	double calculatedMedian = tmpMedian / 100.0;
	CPPUNIT_ASSERT_MESSAGE("Calculated median grayvalue is not equal to the desired value.", testMedian == calculatedMedian);
}

void mitkImageStatisticsCalculatorTestSuite::VerifyStatistics(mitk::ImageStatisticsContainer::StatisticsObject stats,
	unsigned long N,
	double mean,
	double MPP,
	//double median,
	double skewness,
	double kurtosis,
	//double uniformity,
	//double UPP,
	double variance,
	double stdev,
	double min,
	double max,
	double RMS,
	//double entropy,
	vnl_vector<int> minIndex,
	vnl_vector<int> maxIndex)
{
	auto numberOfVoxelsObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::VoxelCountType>(mitk::ImageStatisticsConstants::NUMBEROFVOXELS());
	auto meanObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEAN());
	auto mppObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MPP());
	//auto medianObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEDIAN());
	auto skewnessObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::SKEWNESS());
	auto kurtosisObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::KURTOSIS());
	//auto uniformityObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::UNIFORMITY());
	//auto uppObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::UPP());
	auto varianceObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::VARIANCE());
	auto standardDeviationObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());
	auto minObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MINIMUM());
	auto maxObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MAXIMUM());
	auto rmsObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::RMS());
	//auto entropyObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::ENTROPY());
	auto minIndexObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::IndexType>(mitk::ImageStatisticsConstants::MINIMUMPOSITION());
	auto maxIndexObject = stats.GetValueConverted<mitk::ImageStatisticsContainer::IndexType>(mitk::ImageStatisticsConstants::MAXIMUMPOSITION());

	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", numberOfVoxelsObject - N == 0);
	MITK_INFO << "Difference in mean value: " << std::abs(meanObject - mean);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(meanObject - mean) < mitk::eps);
	// in two test cases MPP is None because the roi has no positive pixels
	if (!std::isnan(mppObject))
	{
		CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(mppObject - MPP) < mitk::eps);
	}
	//CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(medianObject - median) < mitk::eps);
	MITK_INFO << "Difference in skewness value: " << std::abs(skewnessObject - skewness);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(skewnessObject - skewness) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(kurtosisObject - kurtosis) < mitk::eps);
	//CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(uniformityObject - uniformity) < mitk::eps);
	//CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(uppObject - UPP) < mitk::eps);
	MITK_INFO << "Difference in variance value: " << std::abs(varianceObject - variance);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(varianceObject - variance) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(standardDeviationObject - stdev) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(minObject - min) < mitk::eps);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(maxObject - max) < mitk::eps);
	MITK_INFO << "Difference in RMS value: " << std::abs(rmsObject - RMS);
	CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(rmsObject - RMS) < mitk::eps);
	//CPPUNIT_ASSERT_MESSAGE("Calculated value does not fit expected value", std::abs(entropyObject - entropy) < mitk::eps);
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