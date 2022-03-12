/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "itkMultiOutputNaryFunctorImageFilter.h"

#include "mitkTestingMacros.h"
#include "mitkVector.h"

#include "mitkTestDynamicImageGenerator.h"

class TestFunctor
{
public:
  typedef std::vector<int> InputPixelArrayType;
  typedef std::vector<int> OutputPixelArrayType;
  typedef itk::Index<2> IndexType;

  TestFunctor()
  {
    secondOutputSelection = 0;
  };

  ~TestFunctor() {};

  int secondOutputSelection;

  unsigned int GetNumberOfOutputs() const
  {
    return 4;
  }

  bool operator!=( const TestFunctor & other) const
  {
    return !(*this == other);
  }

  bool operator==( const TestFunctor & other ) const
  {
    return secondOutputSelection == other.secondOutputSelection;
  }

  inline OutputPixelArrayType operator()( const InputPixelArrayType & value, const IndexType& currentIndex ) const
  {
    OutputPixelArrayType result;

    int sum = 0;
    for (InputPixelArrayType::const_iterator pos = value.begin(); pos != value.end(); ++pos)
    {
      sum += *pos;
    }

    result.push_back(sum);
    result.push_back(value[secondOutputSelection]);
    result.push_back(currentIndex[0]);
    result.push_back(currentIndex[1]);

    return result;
  }
};

int itkMultiOutputNaryFunctorImageFilterTest(int  /*argc*/, char*[] /*argv[]*/)
{
  // always start with this!
  MITK_TEST_BEGIN("itkMultiOutputNaryFunctorImageFilter")

    //Prepare test artifacts and helper

    mitk::TestImageType::Pointer img1 = mitk::GenerateTestImage();
  mitk::TestImageType::Pointer img2 = mitk::GenerateTestImage(10);
  mitk::TestImageType::Pointer img3 = mitk::GenerateTestImage(100);

  mitk::TestImageType::IndexType testIndex1;
  testIndex1[0] =   0;
  testIndex1[1] =   0;

  mitk::TestImageType::IndexType testIndex2;
  testIndex2[0] =   2;
  testIndex2[1] =   0;

  mitk::TestImageType::IndexType testIndex3;
  testIndex3[0] =   0;
  testIndex3[1] =   1;

  mitk::TestImageType::IndexType testIndex4;
  testIndex4[0] =   1;
  testIndex4[1] =   1;

  mitk::TestImageType::IndexType testIndex5;
  testIndex5[0] =   2;
  testIndex5[1] =   2;

  //Test default usage of filter
  typedef itk::MultiOutputNaryFunctorImageFilter<mitk::TestImageType,mitk::TestImageType,TestFunctor> FilterType;
  FilterType::Pointer testFilter = FilterType::New();

  testFilter->SetInput(0,img1);
  testFilter->SetInput(1,img2);
  testFilter->SetInput(2,img3);

  testFilter->SetNumberOfWorkUnits(2);

  testFilter->Update();

  mitk::TestImageType::Pointer out1 = testFilter->GetOutput(0);
  mitk::TestImageType::Pointer out2 = testFilter->GetOutput(1);
  mitk::TestImageType::Pointer out3 = testFilter->GetOutput(2);
  mitk::TestImageType::Pointer out4 = testFilter->GetOutput(3);

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #1 (functor #1)",111 == out1->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #2 (functor #1)",333 == out1->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #3 (functor #1)",444 == out1->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #4 (functor #1)",555 == out1->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #5 (functor #1)",999 == out1->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #1 (functor #1)",1 == out2->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #2 (functor #1)",3 == out2->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #3 (functor #1)",4 == out2->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #4 (functor #1)",5 == out2->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #5 (functor #1)",9 == out2->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #1 (functor #1)",0 == out3->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #2 (functor #1)",2 == out3->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #3 (functor #1)",0 == out3->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #4 (functor #1)",1 == out3->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #5 (functor #1)",2 == out3->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #1 (functor #1)",0 == out4->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #2 (functor #1)",0 == out4->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #3 (functor #1)",1 == out4->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #4 (functor #1)",1 == out4->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #5 (functor #1)",2 == out4->GetPixel(testIndex5));

  //Test with functor set by user
  TestFunctor funct2;
  funct2.secondOutputSelection = 1;

  testFilter->SetFunctor(funct2);

  testFilter->Update();

  out1 = testFilter->GetOutput(0);
  out2 = testFilter->GetOutput(1);
  out3 = testFilter->GetOutput(2);
  out4 = testFilter->GetOutput(3);

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #1 (functor #2)",111 == out1->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #2 (functor #2)",333 == out1->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #3 (functor #2)",444 == out1->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #4 (functor #2)",555 == out1->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #1 index #5 (functor #2)",999 == out1->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #1 (functor #2)",10 == out2->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #2 (functor #2)",30 == out2->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #3 (functor #2)",40 == out2->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #4 (functor #2)",50 == out2->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #2 index #5 (functor #2)",90 == out2->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #1 (functor #2)",0 == out3->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #2 (functor #2)",2 == out3->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #3 (functor #2)",0 == out3->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #4 (functor #2)",1 == out3->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #3 index #5 (functor #2)",2 == out3->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #1 (functor #2)",0 == out4->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #2 (functor #2)",0 == out4->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #3 (functor #2)",1 == out4->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #4 (functor #2)",1 == out4->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of output #4 index #5 (functor #2)",2 == out4->GetPixel(testIndex5));

  //Test with mask set
  mitk::TestMaskType::Pointer mask = mitk::GenerateTestMask();
  testFilter->SetMask(mask);

  testFilter->Update();

  out1 = testFilter->GetOutput(0);
  out2 = testFilter->GetOutput(1);
  out3 = testFilter->GetOutput(2);
  out4 = testFilter->GetOutput(3);

  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #1 index #1 (functor #2)",0 == out1->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #1 index #2 (functor #2)",333 == out1->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #1 index #3 (functor #2)",444 == out1->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #1 index #4 (functor #2)",0 == out1->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #1 index #5 (functor #2)",0 == out1->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #2 index #1 (functor #2)",0 == out2->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #2 index #2 (functor #2)",30 == out2->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #2 index #3 (functor #2)",40 == out2->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #2 index #4 (functor #2)",0 == out2->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #2 index #5 (functor #2)",0 == out2->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #3 index #1 (functor #2)",0 == out3->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #3 index #2 (functor #2)",2 == out3->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #3 index #3 (functor #2)",0 == out3->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #3 index #4 (functor #2)",0 == out3->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #3 index #5 (functor #2)",0 == out3->GetPixel(testIndex5));

  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #4 index #1 (functor #2)",0 == out4->GetPixel(testIndex1));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #4 index #2 (functor #2)",0 == out4->GetPixel(testIndex2));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #4 index #3 (functor #2)",1 == out4->GetPixel(testIndex3));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #4 index #4 (functor #2)",0 == out4->GetPixel(testIndex4));
  CPPUNIT_ASSERT_MESSAGE("Check pixel of masked output #4 index #5 (functor #2)",0 == out4->GetPixel(testIndex5));

  MITK_TEST_END()
}
