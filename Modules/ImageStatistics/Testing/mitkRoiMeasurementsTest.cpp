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
#include "mitkTestingMacros.h"
#include "mitkDicomSeriesReader.h"
#include <mitkPlanarFigure.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarSubdivisionPolygon.h>
#include "itkTimeProbesCollectorBase.h"

/**
 * \brief Test for mitkRoiMeasurements
 *
 *  This test reads two different input-images and generates for each a defined ROI. After that the ImageStatisticsCalculator
 *  computes mean-value and standard deviation within the ROI. The area is taken by the PlanarFigure itself.
 *  In the end the calculated values are compared with known values to verify that all values are calculated correctly.
 *
 *  In the first part of the test the statistics within a PlanarPolygon are checked, in the second part a PlanarSubdivisionPolygon
 *  is used.
 *
 */
class mitkRoiMeasurementsTestClass
{

public:

  /**
    \brief Creates and returns a ROI in form of a PlanarPolygon with four corners.
  */

  static mitk::PlanarFigure::Pointer CreatePolygonROI(mitk::Image* image, mitk::Point2D p0, mitk::Point2D p1, mitk::Point2D p2, mitk::Point2D p3)
  {
    mitk::PlanarFigure::Pointer result;
    mitk::PlanarPolygon::Pointer polygon = mitk::PlanarPolygon::New();
    polygon->SetGeometry2D(image->GetSlicedGeometry()->GetGeometry2D(0));
    polygon->AddControlPoint(p0);
    polygon->AddControlPoint(p1);
    polygon->AddControlPoint(p2);
    polygon->AddControlPoint(p3);
    polygon->SetClosed(true);
    polygon->Initialize();
    polygon->EvaluateFeatures();
    result = polygon;

    return result;
  }

  /**
    \brief Creates and returns a ROI in form of a PlanarSubdivisionPolygon. The PlanarSubdivisionPolygon represents a
    smooth circular ROI.
  */
  static mitk::PlanarFigure::Pointer CreateSubdivisionPolygonROI(mitk::Image* image)
  {
    mitk::PlanarFigure::Pointer result;
    mitk::PlanarSubdivisionPolygon::Pointer subdivisionPolygon = mitk::PlanarSubdivisionPolygon::New();
    subdivisionPolygon->SetGeometry2D(image->GetSlicedGeometry()->GetGeometry2D(0));

    mitk::Point2D origin; origin[0] = 299.5; origin[1] = 299.5;
    double radius = 290.0;

    double angleStep = 2.0 * vnl_math::pi / 20;
    for(double angle = 0.1 ; angle < 1.98 * vnl_math::pi; angle += angleStep)
    {
      mitk::Point2D point;
      point[0] = origin[0] + radius * cos(angle);
      point[1] = origin[1] + radius * sin(angle);

      subdivisionPolygon->AddControlPoint(point);
    }

    subdivisionPolygon->SetClosed(true);

    subdivisionPolygon->Initialize();
    subdivisionPolygon->EvaluateFeatures();

    result = subdivisionPolygon;

    return result;
  }

   /**
    \brief Creates an instance of ImageStatisticsCalculator and computes the statistics of the given input image and PlanarFigure.
    Returns the calculated statistics.
  */
  static mitk::ImageStatisticsCalculator::Statistics CalculateStatistics(mitk::Image* image, mitk::PlanarFigure* figure)
  {
    mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
    mitk::ImageStatisticsCalculator::Statistics result;

    statisticsCalculator->SetImage(image);
    statisticsCalculator->SetMaskingModeToPlanarFigure();
    statisticsCalculator->SetPlanarFigure(figure);
    statisticsCalculator->ComputeStatistics();

    result = statisticsCalculator->GetStatistics(0);
    return result;
  }

  /**
    \brief Compares calculated statistics with known values. Let tests fail if any calculated value does not equal with the known
    statistics.
  */
  static void CompareValues(mitk::ImageStatisticsCalculator::Statistics statistics,
    mitk::PlanarFigure* figure,
    double expectedArea, double expectedSigma, double expectedMean, double areaEps)
  {
    MITK_TEST_CONDITION( fabs(statistics.Mean - expectedMean) < 0.5,
                         "Mean is close enough to " << expectedMean << " (actual value " << statistics.Mean << ")" );

    MITK_TEST_CONDITION( fabs(statistics.Sigma - expectedSigma) < 0.5,
                         "Sigma is close enough to " << expectedSigma << " (actual value " << statistics.Sigma << ")" );

    MITK_TEST_CONDITION( fabs( figure->GetQuantity(1) - expectedArea) < areaEps,
                         "Area is close enough to " << expectedArea << " (actual value " << figure->GetQuantity(1) << ")" );
  }
};

/**
  \brief Verifies the correct calculation of mean densitiy and standard deviation within a PlanarFigure. Also compares the area
  of the PlanarFigure with known values.

*/
int mitkRoiMeasurementsTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkRoiMeasurementsTest")

  try
  {
    MITK_TEST_CONDITION_REQUIRED(argc == 2, "Path to testimages is defined.");
    std::string path = argv[1];

    mitk::DicomSeriesReader::StringContainer fileGrayvalues;
    std::string filenameGrayvalues(path + "grayvalues.dcm");
    MITK_TEST_OUTPUT(<<"Loading rectangles test image from "<<filenameGrayvalues)

    fileGrayvalues.push_back(filenameGrayvalues);
    mitk::Image::Pointer testImageGrayvalues;
    mitk::DicomSeriesReader::FileNamesGrouping seriesInFiles = mitk::DicomSeriesReader::GetSeries( fileGrayvalues, true );
    mitk::DataNode::Pointer imageNode = mitk::DicomSeriesReader::LoadDicomSeries(fileGrayvalues);
    testImageGrayvalues = dynamic_cast<mitk::Image*>(imageNode->GetData());

    MITK_INFO << "Mean density tests for ROI tool";
    MITK_INFO << "Test 1:";

    // Do not change values!
    mitk::Point2D p0; p0[0] = 36.5; p0[1] = 2.5;
    mitk::Point2D p1; p1[0] = 38.5; p1[1] = 2.5;
    mitk::Point2D p2; p2[0] = 38.5; p2[1] = 36.5;
    mitk::Point2D p3; p3[0] = 36.5; p3[1] = 36.5;

    mitk::PlanarFigure::Pointer figure = mitkRoiMeasurementsTestClass::CreatePolygonROI( testImageGrayvalues, p0, p1, p2, p3 );
    mitk::ImageStatisticsCalculator::Statistics statistics = mitkRoiMeasurementsTestClass::CalculateStatistics(testImageGrayvalues, figure);

    mitkRoiMeasurementsTestClass::CompareValues(statistics, figure, 68.0, 49.8, 103.8, 0.5);

    MITK_INFO << "Test 2:";

    // Do not change values!
    p0[0] = 0.5; p0[1] = 2.5;
    p1[0] = 2.5; p1[1] = 2.5;
    p2[0] = 2.5; p2[1] = 36.5;
    p3[0] = 0.5; p3[1] = 36.5;

    figure = mitkRoiMeasurementsTestClass::CreatePolygonROI( testImageGrayvalues, p0, p1, p2, p3 );
    statistics = mitkRoiMeasurementsTestClass::CalculateStatistics(testImageGrayvalues, figure);

    mitkRoiMeasurementsTestClass::CompareValues(statistics, figure, 68.0, 56.0, 88.4, 0.5);

    MITK_INFO << "Test 3:";

    // Do not change values!
    p0[0] = 2.5; p0[1] = 2.5;
    p1[0] = 36.5; p1[1] = 2.5;
    p2[0] = 36.5; p2[1] = 36.5;
    p3[0] = 2.5; p3[1] = 36.5;

    figure = mitkRoiMeasurementsTestClass::CreatePolygonROI( testImageGrayvalues, p0, p1, p2, p3 );
    statistics = mitkRoiMeasurementsTestClass::CalculateStatistics(testImageGrayvalues, figure);

    mitkRoiMeasurementsTestClass::CompareValues(statistics, figure, 1156.0, 56.2, 98.2, 0.5);

    MITK_INFO << "Test 4:";

    // Do not change values!
    p0[0] = 10.5; p0[1] = 11.5;
    p1[0] = 28.5; p1[1] = 11.5;
    p2[0] = 28.5; p2[1] = 27.5;
    p3[0] = 10.5; p3[1] = 27.5;

    figure = mitkRoiMeasurementsTestClass::CreatePolygonROI( testImageGrayvalues, p0, p1, p2, p3 );

    statistics = mitkRoiMeasurementsTestClass::CalculateStatistics(testImageGrayvalues, figure);

    mitkRoiMeasurementsTestClass::CompareValues(statistics, figure, 288.0, 28.0, 100.4, 0.5);

    mitk::DicomSeriesReader::StringContainer fileCircle;
    std::string filenameCircle(path + "circle.dcm");
    MITK_TEST_OUTPUT(<<"Loading gradient circle test image from "<<filenameCircle)

    fileCircle.push_back(filenameCircle);
    mitk::Image::Pointer testImageCircle;
    seriesInFiles = mitk::DicomSeriesReader::GetSeries( fileCircle, true );
    imageNode = mitk::DicomSeriesReader::LoadDicomSeries(fileCircle);
    testImageCircle = dynamic_cast<mitk::Image*>(imageNode->GetData());

    MITK_INFO << "Circle for smooth ROI";

    figure = mitkRoiMeasurementsTestClass::CreateSubdivisionPolygonROI( testImageCircle);
    statistics = mitkRoiMeasurementsTestClass::CalculateStatistics(testImageCircle, figure);

    mitkRoiMeasurementsTestClass::CompareValues(statistics, figure, 264207.7, 33.8, 95.6, 1000.0);

  }
  catch (std::exception& e)
  {
    std::cout << "Error: " <<  e.what() << std::endl;
  }

  MITK_TEST_END()
}
