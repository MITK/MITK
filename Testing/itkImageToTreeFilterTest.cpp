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
#include <itkStartPointData.h>

#include <mitkVesselTreeData.h>

// image type
typedef unsigned char   PixelType;
const unsigned int      Dimension = 3;
typedef itk::Image<PixelType, Dimension>            ImageType;
typedef ImageType::Pointer                          ImagePointer;
typedef ImageType::PointType                        PointType;
typedef ImageType::DirectionType                    DirectionType;

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
typedef FilterContextType::StartPointQueueType      StartPointQueueType;
typedef itk::StartPointData<ImageType>              StartPointDataType;
typedef StartPointDataType::Pointer                 StartPointDataPointer;

typedef std::list<int>                              ResultListType;

/******************************************************************
 * TEST 1: Saving and loading data objects to the filter context
 *****************************************************************/
int testFilterContext(char* argv[])
{
  // init some test data
  PointType testPoint1;
  testPoint1.Fill(0);
  DirectionType testDirection1;
  testDirection1.Fill(0);
  StartPointDataPointer data1 = StartPointDataType::New();
  data1->SetStartPoint(&testPoint1);
  data1->SetStartDirection(&testDirection1);

  PointType testPoint2;
  testPoint2.Fill(1);
  DirectionType testDirection2;
  testDirection2.Fill(0);
  StartPointDataPointer data2 = StartPointDataType::New();
  data2->SetStartPoint(&testPoint2);
  data2->SetStartDirection(&testDirection2);


  std::cout << " *** Testing ITTFilterContext for storage of objects ***\n";
  FilterContextPointer filterContext = FilterContextType::New();

  StartPointQueueType* pointQueue1 = filterContext->GetStartPointQueue();
  std::cout << "Pushing points to filter context...\n";
  pointQueue1->push(data1);
  pointQueue1->push(data2);

  std::cout << "Reading points from filter context...\n";
  StartPointQueueType* pointQueue2 = filterContext->GetStartPointQueue();
  if (pointQueue2->front() == data1)
  {
    pointQueue2->pop();
  }
  else
  {
    std::cout << "Points not in queue.\n";
    std::cout << "[TEST FAILED]\n";
    return EXIT_FAILURE;
  }

  if (pointQueue2->front() == data2)
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
 // TODO: test init of image
int testInitFilter(char* argv[])
{
    // init some test data
  PointType testPoint1;
  testPoint1.Fill(0);

  DirectionType testDirection1;
  testDirection1.Fill(0);

  std::cout << " *** Testing initialization of filter ***\n";
  std::cout << "Loading new StartPointData to filter...\n";
  ImageToTreeFilterPointer testFilter = ImageToTreeFilterType::New();
  testFilter->SetStartPoint(&testPoint1);
  testFilter->SetStartDirection(&testDirection1);

  // start point should be the first point in the filter
  std::cout << "Reading StartPointData from filter...\n";
  FilterContextPointer testFilterContext = testFilter->GetFilterContext();
  StartPointQueueType* testQueue = testFilterContext->GetStartPointQueue();
  StartPointDataPointer testData = testQueue->front();
  PointType* testPoint2 = testData->GetStartPoint();
  DirectionType* testDirection2 = testData->GetStartDirection();

  if(testPoint1 != *testPoint2)
  {
    std::cout << "Startpoint not in queue.\n";
    std::cout << " *** [TEST FAILED] ***\n";
    return EXIT_FAILURE;
  }

  if(testDirection1 != *testDirection2)
  {
    std::cout << "Startdirection not in queue.\n";
    std::cout << " *** [TEST FAILED] ***\n";
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
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

  failRatio = 100 * (float) failedCount / (float) testCount;

  std::cout << "Result: ";
  std::cout << failedCount;
  std::cout << "/";
  std::cout << testCount;
  std::cout << " Tests failed (";
  std::cout << failRatio;
  std::cout << " %)\n";

  return EXIT_SUCCESS;
}
