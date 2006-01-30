/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <itkImage.h>
#include <itkITTFilterContext.h>
#include <itkImageToTreeFilter.h>

#include <mitkVesselTreeData.h>

int itkImageToTreeFilterTest(int, char* argv[] )
{
  // image type
  typedef unsigned char   PixelType;
  const unsigned int      Dimension = 3;
  typedef itk::Image<PixelType, Dimension>            ImageType;
  typedef ImageType::Pointer                          ImagePointer;
  typedef ImageType::PointType                        PointType;

  // tree type
  typedef mitk::VesselTreeData                        OutputTreeType;
  typedef OutputTreeType::Pointer                     OutputTreePointer;



//   typedef itk::ImageFileReader<ImageType>         ImageReaderType;
//   typedef ImageReaderType::Pointer                ImageReaderPointer;

  // test classes
  typedef itk::ImageToTreeFilter<ImageType, OutputTreeType>
                                                      ImageToTreeFilterType;
  typedef ImageToTreeFilterType::Pointer              ImageToTreeFilterPointer;
  typedef itk::ITTFilterContext<ImageType, OutputTreeType>
                                                      FilterContextType;
  typedef FilterContextType::Pointer                  FilterContextPointer;
  typedef FilterContextType::PointQueueType           PointQueueType;

  // init some test data
  PointType testPoint1;
  testPoint1.Fill(0);

  PointType testPoint2;
  testPoint1.Fill(1);

  // TODO: read files with mitk and convert to itk
  // read in first file
//   ImageReaderPointer reader = ImageReaderType::New();
//   reader->SetFileName(argv[1]);
//   reader->Update();
//
//   ImagePointer testImage = reader->GetOutput();
//   ImagePointer testImage2;

  /******************************************************************
   * TEST 1: Saving and loading data objects to the filter context
  *****************************************************************/
  std::cout << "Testing ITTFilterContext for storage of objects...\n";
  FilterContextPointer filterContext = FilterContextType::New();

  PointQueueType* pointQueue1 = filterContext->GetStartPointQueue();
  std::cout << "Pushing points to filter context...\n";
  pointQueue1->push(testPoint1);
  pointQueue1->push(testPoint2);

  std::cout << "Reading points from filter context...\n";
  PointQueueType* pointQueue2 = filterContext->GetStartPointQueue();
  if (pointQueue2->front() == testPoint1)
  {
    pointQueue2->pop();
  }
  else
  {
    std::cout << "Points not in queue.\n";
    std::cout << "[TEST FAILED]\n";
    return EXIT_FAILURE;
  }

  if (pointQueue2->front() == testPoint2)
  {
    pointQueue2->pop();
  }
  else
  {
    std::cout << "Points not in queue.\n";
    std::cout << "[TEST FAILED]\n";
    return EXIT_FAILURE;
  }

  std::cout << "[TEST PASSED]\n";

  /****************************************************************
   * TEST 2: Initialising the filter
   ****************************************************************/
  std::cout << "Testing initialization of filter...\n";
  ImageToTreeFilterPointer testFilter = ImageToTreeFilterType::New();
  testFilter->SetStartPoint(&testPoint1);

  // start point should be the first point in the filter
  FilterContextPointer testFilterContext = testFilter->GetFilterContext();
  PointQueueType* testQueue = filterContext->GetStartPointQueue();
  if(testQueue->front() != testPoint1)
  {
    std::cout << "Startpoint not in queue.\n";
    std::cout << "[TEST FAILED]\n";
    return EXIT_FAILURE;
  }

  std::cout << "[TEST DONE]\n";
  return EXIT_SUCCESS;
}

