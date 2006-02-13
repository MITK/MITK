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

#include <fstream>
#include <list>
#include <sstream>
#include <string>

#include <itkImage.h>
#include <itkTreeContainer.h>

#include <itkImageToTreeFilter.h>
#include <itkITTFilterContext.h>
#include <itkRegistrationModelXMLWriter.h>
#include <itkStartPointData.h>
#include <itkTubeSegmentModel.h>



// image type
typedef double                                        PixelType;
const unsigned int                                    Dimension = 3;

typedef itk::Image<PixelType, Dimension>              ImageType;
typedef ImageType::Pointer                            ImagePointer;
typedef ImageType::PointType                          PointType;
typedef ImageType::DirectionType                      DirectionType;

typedef itk::RegistrationModel<PixelType>             RegistrationModelType;
typedef RegistrationModelType::Pointer                RegistrationModelPointer;

// tree type
typedef itk::TreeContainer<RegistrationModelPointer>  OutputTreeType;
typedef OutputTreeType::Pointer                       OutputTreePointer;

// test classes
typedef itk::ImageToTreeFilter<ImageType, OutputTreeType>
                                                      ImageToTreeFilterType;
typedef ImageToTreeFilterType::Pointer                ImageToTreeFilterPointer;
typedef itk::ITTFilterContext<ImageType, OutputTreeType>
                                                      FilterContextType;
typedef FilterContextType::Pointer                    FilterContextPointer;
typedef FilterContextType::StartPointQueueType        StartPointQueueType;
typedef itk::StartPointData<ImageType>                StartPointDataType;
typedef StartPointDataType::Pointer                   StartPointDataPointer;

typedef itk::TubeSegmentModel<PixelType>              TubeSegmentModelType;
typedef TubeSegmentModelType::Pointer                 TubeSegmentModelPointer;

typedef itk::RegistrationModelXMLWriter<TubeSegmentModelType>
                                                      RegistrationModelWriterType;
typedef RegistrationModelWriterType::Pointer          RegistrationModelWriterPointer;

typedef std::list<int>                                ResultListType;

/******************************************************************
 * TEST 1: Saving and loading data objects to the filter context
 *****************************************************************/
int testFilterContext()
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
  return EXIT_SUCCESS;
}

/****************************************************************
 * TEST 2: Initialising the filter
 ****************************************************************/
 // TODO: test init of image
int testInitFilter()
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

/****************************************************************
 * TEST 3: XML Writer
 ****************************************************************/
int testRegistrationModelXMLWriter()
{
  std::cout << " *** Testing the RegistrationModelXMLWriter ***\n";

  typedef TubeSegmentModelType::PointSetType          PointSetType;
  typedef PointSetType::Pointer                       PointSetPointer;
  typedef PointSetType::PointType                     PointType;
  typedef PointSetType::PointsContainer               PointsContainerType;
  typedef PointsContainerType::Pointer                PointsContainerPointer;
  typedef PointsContainerType::ElementIdentifier      ElementIdentifier;
  typedef PointSetType::PointDataContainer            PointDataContainerType;
  typedef PointDataContainerType::Pointer             PointDataContainerPointer;

  PixelType pixelValue = 255;

  TubeSegmentModelPointer tubeSegment = TubeSegmentModelType::New();
  ElementIdentifier index = 0;
  PointSetPointer pointSet = PointSetType::New();
  PointsContainerPointer pointsContainer = PointsContainerType::New();
  PointDataContainerPointer pointDataContainer = PointDataContainerType::New();

  // init test data
  PointType startPoint;
  startPoint.Fill(0);
  tubeSegment->SetStartPoint(&startPoint);

  PointType point1; // 1, 1, 0
  point1[0] = 1;
  point1[1] = 1;
  point1[2] = 0;
  pointsContainer->InsertElement(index, point1);
  pointDataContainer->InsertElement(index, pixelValue);
  index++;

  PointType point2; // 1, 0, 1
  point2[0] = 1;
  point2[1] = 0;
  point2[2] = 1;
  pointsContainer->InsertElement(index, point2);
  pointDataContainer->InsertElement(index, pixelValue);
  index++;

  PointType point3; // 1, 0, 1
  point3[0] = 1;
  point3[1] = 0;
  point3[2] = 1;
  pointsContainer->InsertElement(index, point3);
  pointDataContainer->InsertElement(index, pixelValue);
  index++;

  pointSet->SetPoints(pointsContainer);
  pointSet->SetPointData(pointDataContainer);
  tubeSegment->SetPointSet(pointSet);

  RegistrationModelWriterPointer modelWriter = RegistrationModelWriterType::New();
  modelWriter->SetRegistrationModel(tubeSegment);
  modelWriter->SetFilename("test.xml");
  modelWriter->WriteFile();

  // compare to normal data
  std::ifstream file("test.xml");
  std::stringstream buffer;

  char charBuffer;
  while (file.get(charBuffer))
  {
    buffer.put(charBuffer);
  }

  if (!file.eof() || buffer.bad())
  {
    std::cout << "Error reading the testfile.";
  }

  // TODO: find a better way to compare the strings
  std::string value = buffer.str();
  std::string expected = "<model>\n  <name>TubeSegment</name>\n  <startPoint>\n    <point>\n      <x>0</x>\n      <y>0</y>\n      <z>0</z>\n      </point>\n      </startPoint>\n      <points>\n      <point>\n      <x>1</x>\n      <y>1</y>\n      <z>0</z>\n      <value>1</value>\n      </point>\n      <point>\n      <x>1</x>\n      <y>0</y>\n      <z>1</z>\n      <value>1</value>\n      </point>\n      <point>\n      <x>1</x>\n      <y>0</y>\n      <z>1</z>\n      <value>1</value>\n      </point>\n      </points>\n      </model>\n";


  if (value != expected)
  {
    std::cout << "XML does not match!\n";
    std::cout << value;
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}



int itkImageToTreeFilterTest(int i, char* argv[] )
{
  ResultListType resultList;
  int failedCount = 0;
  int testCount;
  float failRatio;

  // run all tests
  resultList.push_back(testFilterContext());
  resultList.push_back(testInitFilter());
//   resultList.push_back(testRegistrationModelXMLWriter());

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
