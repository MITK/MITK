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

#include "mitkStandardFileLocations.h"
#include "mitkDicomSeriesReader.h"
#include "mitkTestingMacros.h"
#include "mitkImageStatisticsCalculator.h"
#include "mitkPlanarPolygon.h"

#include "mitkDicomSeriesReader.h"
#include <itkGDCMSeriesFileNames.h>

#include "vtkStreamingDemandDrivenPipeline.h"

#include <mitkDataNodeFactory.h>
#include <mitkStandaloneDataStorage.h>

#include "itkImage.h"
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkSimilarityIndexImageFilter.h>
#include <itkImageFileWriter.h>

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
      * half pixel in diagonal-direction (white)
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
      * one pixel (white) + 2 half pixels (white) + 1 half pixel (black)
      * -> mean of 191.25 expected
      ******************************/
      mitk::PlanarPolygon::Pointer figure1 = mitk::PlanarPolygon::New();
      figure1->SetGeometry2D( geom );
      mitk::Point2D pnt1; pnt1[0] = 1.1; pnt1[1] = 1.1;
      figure1->PlaceFigure( pnt1 );

      mitk::Point2D pnt2; pnt2[0] = 2.0; pnt2[1] = 2.0;
      figure1->SetControlPoint( 1, pnt2, true );
      mitk::Point2D pnt3; pnt3[0] = 3.0; pnt3[1] = 1.0;
      figure1->SetControlPoint( 2, pnt3, true );
      mitk::Point2D pnt4; pnt4[0] = 2.0; pnt4[1] = 0.0;
      figure1->SetControlPoint( 3, pnt4, true );
      figure1->GetPolyLine(0);

      testCase test;
      test.id = testCases.size();
      test.figure = figure1;
      test.mean = 191.25;
      test.sd = 127.5;

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
      test.mean = 191.50;
      test.sd = 89.80;

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
      test.sd = 89.80;

      testCases.push_back( test );
    }

    {
      /*****************************
      * half pixel (black) + whole pixel (white) + half pixel (gray) in x-direction
      * -> mean of 127.66 expected
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
      test.mean = 127.66;
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
      test.sd = 89.80;

      testCases.push_back( test );
    }

    {
      /*****************************
      * 2 whole pixel (white) + 2 whole pixel (black) in y-direction
      * -> mean of 127.66 expected
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
      test.mean = 127.66;
      test.sd = 127.5;

      testCases.push_back( test );
    }


    {
      /*****************************
      * 9 whole pixels (white) + 3 half pixels (white)
      * + 3 whole pixel (black) [ + 3 slightly less than half pixels (black)]
      * -> mean of 204.0 expected
      ******************************/
      mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
      figure2->SetGeometry2D( geom );
      mitk::Point2D pnt1; pnt1[0] = 0.5; pnt1[1] = 0.5;
      figure2->PlaceFigure( pnt1 );

      mitk::Point2D pnt2; pnt2[0] = 3.5; pnt2[1] = 3.5;
      figure2->SetControlPoint( 1, pnt2, true );
      mitk::Point2D pnt3; pnt3[0] = 8.4999; pnt3[1] = 3.5;
      figure2->SetControlPoint( 2, pnt3, true );
      mitk::Point2D pnt4; pnt4[0] = 5.4999; pnt4[1] = 0.5;
      figure2->SetControlPoint( 3, pnt4, true );
      figure2->GetPolyLine(0);

      testCase test;
      test.id = testCases.size();
      test.figure = figure2;
      test.mean = 204.0;
      test.sd = 105.58;

      testCases.push_back( test );
    }

    {
      /*****************************
      * half pixel (white) + whole pixel (white) + half pixel (black)
      * -> mean of 212.66 expected
      ******************************/
      mitk::PlanarPolygon::Pointer figure2 = mitk::PlanarPolygon::New();
      figure2->SetGeometry2D( geom );
      mitk::Point2D pnt1; pnt1[0] = 9.5; pnt1[1] = 0.5;
      figure2->PlaceFigure( pnt1 );

      mitk::Point2D pnt2; pnt2[0] = 9.5; pnt2[1] = 2.5;
      figure2->SetControlPoint( 1, pnt2, true );
      mitk::Point2D pnt3; pnt3[0] = 11.5; pnt3[1] = 2.5;
      figure2->SetControlPoint( 2, pnt3, true );
      figure2->GetPolyLine(0);

      testCase test;
      test.id = testCases.size();
      test.figure = figure2;
      test.mean = 212.66;
      test.sd = 73.32;

      testCases.push_back( test );
    }
    return testCases;
  }

  static void GetTestConvolutionMask(itk::Image< float, 2 > *mask,
                                      double RadiusInMM)
  {
    /*****************************
      * radius of 2.5 mm
      * -> expected ConvolutionMask:
      * 0,25 0,75 1 0,75 0,25
      * 0,75  1   1   1  0,75
      *  1    1   1   1   1
      * 0,75  1   1   1  0,75
      * 0,25 0,75 1 0,75 0,25
      ******************************/

    typedef itk::Image<float, 2> ImageType;

    ImageType::Pointer convolutionMask = ImageType::New();

    ImageType::RegionType region;
    ImageType::IndexType start;
    ImageType::SizeType size;
    ImageType::IndexType pixelIndex;
    ImageType::SpacingType spacing;
    ImageType::IndexType origin;


    for(unsigned int i = 0; i < 2; ++i)
    {
      spacing[i] = 1.0;
      start[i] = 0.0;

      double countIndex =  2.0 * RadiusInMM / spacing[i];

      //Rounding up to the next integer by cast
      countIndex += 0.9;
      int castedIndex = static_cast<int>(countIndex);

      //We always have an uneven number in size to determine a center-point in the convolution mask
      if(castedIndex % 2 > 0 )
      {
        size[i] = castedIndex;
      }
      else
      {
        size[i] = castedIndex +1;
      }
    }

    region.SetSize(size);
    region.SetIndex(start);

    convolutionMask->SetRegions(region);
    convolutionMask->SetSpacing(spacing);
    convolutionMask->Allocate();

    //y = 0
    pixelIndex[0] = 0;
    pixelIndex[1] = 0;
    convolutionMask->SetPixel(pixelIndex, 0.25);

    pixelIndex[0] = 1;
    pixelIndex[1] = 0;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    pixelIndex[0] = 2;
    pixelIndex[1] = 0;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 3;
    pixelIndex[1] = 0;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    pixelIndex[0] = 4;
    pixelIndex[1] = 0;
    convolutionMask->SetPixel(pixelIndex, 0.25);

    //y = 1
    pixelIndex[1] = 0;
    pixelIndex[1] = 1;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    pixelIndex[0] = 1;
    pixelIndex[1] = 1;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 2;
    pixelIndex[1] = 1;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 3;
    pixelIndex[1] = 1;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 4;
    pixelIndex[1] = 1;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    //y = 2
    pixelIndex[1] = 0;
    pixelIndex[1] = 2;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 1;
    pixelIndex[1] = 2;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 2;
    pixelIndex[1] = 2;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 3;
    pixelIndex[1] = 2;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 4;
    pixelIndex[1] = 2;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    //y = 3
    pixelIndex[1] = 0;
    pixelIndex[1] = 3;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    pixelIndex[0] = 1;
    pixelIndex[1] = 3;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 2;
    pixelIndex[1] = 3;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 3;
    pixelIndex[1] = 3;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 4;
    pixelIndex[1] = 3;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    //y = 4
    pixelIndex[1] = 0;
    pixelIndex[1] = 4;
    convolutionMask->SetPixel(pixelIndex, 0.25);

    pixelIndex[0] = 1;
    pixelIndex[1] = 4;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    pixelIndex[0] = 2;
    pixelIndex[1] = 4;
    convolutionMask->SetPixel(pixelIndex, 1.0);

    pixelIndex[0] = 3;
    pixelIndex[1] = 4;
    convolutionMask->SetPixel(pixelIndex, 0.75);

    pixelIndex[0] = 4;
    pixelIndex[1] = 4;
    convolutionMask->SetPixel(pixelIndex, 0.25);

    typedef itk::ImageRegionConstIteratorWithIndex<ImageType> IteratorType;
    IteratorType it(convolutionMask, convolutionMask->GetLargestPossibleRegion());

    std::cout << std::endl << std::endl;
    std::cout << "Selbsterzeugtes Bild: "<< std::endl;

      //#ifdef DEBUG_SEGMENTATION_CORRECTION
        // Debug Ausgabe:
    unsigned int lastZ = 1000000000;
    unsigned int lastY = 1000000000;
    for(it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      double x = it.Get();
        if (it.GetIndex()[1] != lastY)
        {
          std::cout << std::endl;
          lastY = it.GetIndex()[1];
        }
        if (it.GetIndex()[0] != lastZ)
        {
          std::cout << x << " ";
          lastZ = it.GetIndex()[0];
        }
      }
    //#endif

    typedef itk::SimilarityIndexImageFilter<ImageType, ImageType> FilterType;
    FilterType::Pointer similiarityFilter = FilterType::New();

    similiarityFilter->SetInput1(mask);
    similiarityFilter->SetInput2(convolutionMask);
    similiarityFilter->Update();

    ImageType::Pointer similarityImage = similiarityFilter->GetOutput();
    IteratorType iterator(similarityImage, similarityImage->GetLargestPossibleRegion());

    std::cout << "Bild nach Filter: "<< std::endl;
      //#ifdef DEBUG_SEGMENTATION_CORRECTION
        // Debug Ausgabe:
    lastZ = 1000000000;
    lastY = 1000000000;
    for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
    {
      double x = iterator.Get();
        if (iterator.GetIndex()[1] != lastY)
        {
          std::cout << std::endl;
          lastY = iterator.GetIndex()[1];
        }
        if (iterator.GetIndex()[0] != lastZ)
        {
          std::cout << x << " ";
          lastZ = iterator.GetIndex()[0];
        }
      }
    //#endif

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


int TestUnitilizedImage()
{
  /*****************************
  * loading uninitialized image to datastorage
  ******************************/
  std::cout << "Testing loading uninitialized image to datastorage:";
  try{
    MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception)
    mitk::Image::Pointer image = mitk::Image::New();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(image);

    mitk::ImageStatisticsCalculator::Pointer is = mitk::ImageStatisticsCalculator::New();
    is->ComputeStatistics();
    MITK_TEST_FOR_EXCEPTION_END(mitk::Exception)
  }
  catch (const mitk::Exception&)
  {
    std::cout << "Success: Loading uninitialized image has thrown an exception." << std::endl;
  }

  return 0;
}

int mitkImageStatisticsCalculatorTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkImageStatisticsCalculatorTest")

    //QCoreApplication app(argc, argv);

    mitk::Image::Pointer image = mitkImageStatisticsCalculatorTestClass::GetTestImage();
  MITK_TEST_CONDITION_REQUIRED( image.IsNotNull(), "Loading test image" );

  mitk::Geometry2D::Pointer geom = image->GetSlicedGeometry()->GetGeometry2D(0);

  std::vector<mitkImageStatisticsCalculatorTestClass::testCase> allTestCases =
    mitkImageStatisticsCalculatorTestClass::InitializeTestCases( geom );


  for ( std::vector<mitkImageStatisticsCalculatorTestClass::testCase>::size_type i=0; i<allTestCases.size(); i++ )
  {
    mitkImageStatisticsCalculatorTestClass::testCase test = allTestCases[i];

    const mitk::ImageStatisticsCalculator::Statistics stats =
      mitkImageStatisticsCalculatorTestClass::TestStatistics( image, test.figure );

    int tmpMean = stats.Mean * 100;
    double calculatedMean = tmpMean / 100.0;
    MITK_TEST_CONDITION( calculatedMean == test.mean,
      "Calculated mean grayvalue '"<< calculatedMean <<"'  is equal to the desired value '"
      << test.mean <<"' for testcase #" << test.id );

    int tmpSD = stats.Sigma * 100;
    double calculatedSD = tmpSD / 100.0;
    MITK_TEST_CONDITION( calculatedSD == test.sd,
      "Calculated grayvalue sd '"<< calculatedSD <<"'  is equal to the desired value '"
      << test.sd <<"' for testcase #" << test.id );
  }

  // static method TestHotspotSearch(image):
  // -> formuliert verschiedene TestFälle: Polygon und Masken (Segmentierung)

  TestUnitilizedImage();

  MITK_TEST_END()
}

