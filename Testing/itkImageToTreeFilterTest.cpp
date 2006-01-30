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

#include <list>
#include <itkImage.h>
#include <itkITTFilterContext.h>
#include <itkImageToTreeFilter.h>

#include <mitkVesselTreeData.h>

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

typedef std::list<int>                              ResultListType;

/******************************************************************
 * TEST 1: Saving and loading data objects to the filter context
 *****************************************************************/
int testFilterContext(char* argv[])
{
  // init some test data
  PointType testPoint1;
  testPoint1.Fill(0);

  PointType testPoint2;
  testPoint2.Fill(1);

  std::cout << " *** Testing ITTFilterContext for storage of objects ***\n";
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
    std::cout << " *** [TEST FAILED] ***\n";
    return EXIT_FAILURE;
  }
  std::cout << " *** [TEST PASSED] ***\n";
}

/****************************************************************
 * TEST 2: Initialising the filter
 ****************************************************************/
int testInitFilter(char* argv[])
{
  // init some test data
  PointType testPoint1;
  testPoint1.Fill(0);

  std::cout << " *** Testing initialization of filter ***\n";
  ImageToTreeFilterPointer testFilter = ImageToTreeFilterType::New();
  testFilter->SetStartPoint(&testPoint1);

  // start point should be the first point in the filter
  FilterContextPointer testFilterContext = testFilter->GetFilterContext();
  PointQueueType* testQueue = testFilterContext->GetStartPointQueue();
  if(testQueue->front() != testPoint1)
  {
    std::cout << "Startpoint not in queue.\n";
    std::cout << " *** [TEST FAILED] ***\n";
    return EXIT_FAILURE;
  }
  std::cout << " *** [TEST PASSED] ***\n";
}

int testTubeSegmentDetector(char* argv[] )
{
  return EXIT_FAILURE;
}


int itkImageToTreeFilterTest(int, char* argv[] )
{
  ResultListType resultList;
  int failedCount = 0;
  int testCount;
  float failRatio;

  // run all tests
  resultList.push_back(testFilterContext(argv));
  resultList.push_back(testInitFilter(argv));
//   resultList.push_back(testTubeSegmentDetector(argv));

  std::cout << " *** [ALL TESTS DONE] ***\n";

  testCount = resultList.size();

  while (resultList.size() > 0)
  {
    int value = resultList.front();
    if(value == EXIT_FAILURE) { failedCount++; }
    resultList.pop_front();
  }

  failRatio = (float) failedCount / (float) testCount;

  std::cout << "Result: ";
  std::cout << failedCount;
  std::cout << "/";
  std::cout << testCount;
  std::cout << " Tests failed (";
  std::cout << failRatio;
  std::cout << " %)\n";

  return EXIT_SUCCESS;
}

