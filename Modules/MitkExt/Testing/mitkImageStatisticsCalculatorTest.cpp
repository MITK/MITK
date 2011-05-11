/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkStandardFileLocations.h"
#include "mitkDicomSeriesReader.h"
#include "mitkTestingMacros.h"
#include "mitkImageStatisticsCalculator.h"
#include "mitkPlanarPolygon.h"

#include "mitkDicomSeriesReader.h"
#include <itkGDCMSeriesFileNames.h>


//#include <QtCore>

/**
 * \brief Test class for mitkImageStatisticsCalculator
 *
 * This test covers:
 * - instantiation of an ImageStatisticsCalculator class
 * - correctness of statistics when using PlanarFigures for masking
 */
class mitkImageStatisticsCalculatorTestClass 
{ 

public:

  struct testCase
  {
    int id;
    mitk::PlanarFigure::Pointer figure;
    double mean;
    double sd;
  };


// calculate statistics for the given image and planarpolygon
static const mitk::ImageStatisticsCalculator::Statistics TestStatistics( mitk::Image::Pointer image, mitk::PlanarFigure::Pointer polygon )
{
  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
  statisticsCalculator->SetImage( image );
  statisticsCalculator->SetMaskingModeToPlanarFigure();
  statisticsCalculator->SetPlanarFigure( polygon );

  statisticsCalculator->ComputeStatistics();

  return statisticsCalculator->GetStatistics();
}


// returns a vector of defined test-cases
static std::vector<testCase> InitializeTestCases( mitk::Geometry2D::Pointer geom )
{
  std::vector<testCase> testCases;

  {
    /*****************************
    * one whole white pixel  
    * -> mean of 255 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
    figure1->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 10.5 ; pnt1[1] = 3.5;
    figure1->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
    figure1->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
    figure1->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 10.5; pnt4[1] = 4.5;
    figure1->SetControlPoint( 3, pnt4, true );
    figure1->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure1;
    test.mean = 255.0;
    test.sd = 0.0;

    testCases.push_back( test );
  }

  {
    /*****************************
    * half pixel in x-direction (white) 
    * -> mean of 255 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
    figure1->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 10.0 ; pnt1[1] = 3.5;
    figure1->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
    figure1->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
    figure1->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 10.0; pnt4[1] = 4.5;
    figure1->SetControlPoint( 3, pnt4, true );
    figure1->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure1;
    test.mean = 255.0;
    test.sd = 0.0;

    testCases.push_back( test );
  }


  {
    /*****************************
    * whole pixel (white) + half pixel (gray) in x-direction 
    * -> mean of 191.5 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
    figure1->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
    figure1->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 3.5;
    figure1->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 9.5; pnt3[1] = 4.5;
    figure1->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
    figure1->SetControlPoint( 3, pnt4, true );
    figure1->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure1;
    test.mean = 191.5;
    test.sd = 89.8025;

    testCases.push_back( test );
  }

  {
    /*****************************
    * quarter pixel (black) + whole pixel (white) + half pixel (gray) in x-direction 
    * -> mean of 191.5 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
    figure1->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
    figure1->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 9.25; pnt2[1] = 3.5;
    figure1->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 9.25; pnt3[1] = 4.5;
    figure1->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.5;
    figure1->SetControlPoint( 3, pnt4, true );
    figure1->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure1;
    test.mean = 191.5;
    test.sd = 89.8025;

    testCases.push_back( test );
  }

  {
    /*****************************
    * half pixel (black) + whole pixel (white) + half pixel (gray) in x-direction 
    * -> mean of 127.67 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
    figure1->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 11.0; pnt1[1] = 3.5;
    figure1->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 9.0; pnt2[1] = 3.5;
    figure1->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 9.0; pnt3[1] = 4.0;
    figure1->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 11.0; pnt4[1] = 4.0;
    figure1->SetControlPoint( 3, pnt4, true );
    figure1->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure1;
    test.mean = 127.67;
    test.sd = 127.5;

    testCases.push_back( test );
  }


  {
    /*****************************
    * whole pixel (gray) 
    * -> mean of 128 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
    figure2->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
    figure2->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 11.5;
    figure2->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 11.5;
    figure2->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
    figure2->SetControlPoint( 3, pnt4, true );
    figure2->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure2;
    test.mean = 128.0;
    test.sd = 0.0;

    testCases.push_back( test );
  }

  {
    /*****************************
    * whole pixel (gray) + half pixel (white) in y-direction
    * -> mean of 191.5 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
    figure2->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
    figure2->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 12.0;
    figure2->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 12.0;
    figure2->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
    figure2->SetControlPoint( 3, pnt4, true );
    figure2->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure2;
    test.mean = 191.5;
    test.sd = 89.8025;

    testCases.push_back( test );
  }

  {
    /*****************************
    * whole pixel (gray) + whole pixel (white) + whole pixel (black) in y-direction
    * -> mean of 127.67 expected
    ******************************/
    mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
    figure2->SetGeometry2D( geom );
    mitk::Point2D pnt1; pnt1[0] = 11.5; pnt1[1] = 10.5;
    figure2->PlaceFigure( pnt1 );

    mitk::Point2D pnt2; pnt2[0] = 11.5; pnt2[1] = 13.5;
    figure2->SetControlPoint( 1, pnt2, true );
    mitk::Point2D pnt3; pnt3[0] = 12.5; pnt3[1] = 13.5;
    figure2->SetControlPoint( 2, pnt3, true );
    mitk::Point2D pnt4; pnt4[0] = 12.5; pnt4[1] = 10.5;
    figure2->SetControlPoint( 3, pnt4, true );
    figure2->GetPolyLine(0);

    testCase test;
    test.id = testCases.size();
    test.figure = figure2;
    test.mean = 127.67;
    test.sd = 127.5;

    testCases.push_back( test );
  }


  
  return testCases;
}

// loads the test image
static mitk::Image::Pointer GetTestImage()
{
  mitk::StandardFileLocations::Pointer locator = mitk::StandardFileLocations::GetInstance();

  const std::string filename = locator->FindFile("testimage.dcm", "Modules/MitkExt/Testing/Data");
  if (filename.empty())
  {
    MITK_ERROR << "Could not find test file";
    return NULL;
  }
  else
  {
    MITK_INFO << "Found testimage.dcm";
  }


  itk::FilenamesContainer file;
  file.push_back( filename );

  mitk::DicomSeriesReader* reader = new mitk::DicomSeriesReader;
     
  mitk::DataNode::Pointer node = reader->LoadDicomSeries( file, false, false );
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

  return image;
}

};

int mitkImageStatisticsCalculatorTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkImageStatisticsCalculatorTest")
  
  //QCoreApplication app(argc, argv);

  mitk::Image::Pointer image = mitkImageStatisticsCalculatorTestClass::GetTestImage();
  MITK_TEST_CONDITION_REQUIRED( image.IsNotNull(), "Loading test image" );

  mitk::Geometry2D::Pointer geom = image->GetSlicedGeometry()->GetGeometry2D(0);

  std::vector<mitkImageStatisticsCalculatorTestClass::testCase> allTestCases = mitkImageStatisticsCalculatorTestClass::InitializeTestCases( geom );
  for ( int i=0; i<allTestCases.size(); i++ )
  {
    mitkImageStatisticsCalculatorTestClass::testCase test = allTestCases[i];

    const mitk::ImageStatisticsCalculator::Statistics stats = mitkImageStatisticsCalculatorTestClass::TestStatistics( image, test.figure );

    MITK_TEST_CONDITION( int(stats.Mean) == int(test.mean),
      "Calculated mean grayvalue '"<< stats.Mean <<"'  is equal to the desired value '"
      << test.mean <<"' for testcase #" << test.id );

    MITK_TEST_CONDITION( int(stats.Sigma) == int(test.sd),
      "Calculated grayvalue sd '"<< stats.Sigma <<"'  is equal to the desired value '"
      << test.sd <<"' for testcase #" << test.id );


    //MITK_TEST_CONDITION( int(stats.Sigma) == int(test.sd),
    //  qPrintable( QString("Calculated grayvalue sd '%1'  is equal to the desired value '%2' for testcase #%3")
    //  .arg(stats.Sigma).arg( test.sd ).arg( test.id ) ) );

  }
 

  MITK_TEST_END()
}

