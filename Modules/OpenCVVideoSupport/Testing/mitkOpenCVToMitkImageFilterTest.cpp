/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk includes
#include "mitkOpenCVToMitkImageFilter.h"
#include <mitkStandardFileLocations.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>

/** Documentation
*
* @brief Objects of this class can start an internal thread by calling the Start() method.
*        The thread is then updateing the tested object until the method Stop() is called. The class
*        can be used to test if a filter is thread-save by using multiple objects and let
*        them update simuntanously.
*/
class mitkTestOpenCVToMITKImageFilterThread : public itk::Object
{
public:

  mitkClassMacroItkParent(mitkTestOpenCVToMITKImageFilterThread, itk::Object);
  itkNewMacro(mitkTestOpenCVToMITKImageFilterThread);

  int NumberOfMessages;

protected:

  mitkTestOpenCVToMITKImageFilterThread()
  {
    NumberOfMessages = 0;
  }

  bool ThreadRunning;

  std::thread Thread;

  cv::Mat currentImage;

  mitk::OpenCVToMitkImageFilter::Pointer m_testedFilter;

  void DoSomething()
  {
    while (ThreadRunning)
    {
      m_testedFilter->SetOpenCVMat(currentImage);
      m_testedFilter->Update();
      mitk::Image::Pointer result;
      result = m_testedFilter->GetOutput();
    }
  }


  void ThreadStartTracking()
  {
    this->DoSomething();
  }

public:

  void Start()
  {
    ThreadRunning = true;
    this->Thread = std::thread(&mitkTestOpenCVToMITKImageFilterThread::ThreadStartTracking, this);
  }

  void Stop()
  {
    ThreadRunning = false;
    if (this->Thread.joinable())
      this->Thread.join();
  }

  void setFilter(mitk::OpenCVToMitkImageFilter::Pointer testedFilter)
  {
    m_testedFilter = testedFilter;
  }

  void setImage(cv::Mat image)
  {
    currentImage = image;
  }

};

class mitkOpenCVToMitkImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOpenCVToMitkImageFilterTestSuite);
  MITK_TEST(TestInitialization);
  MITK_TEST(TestThreadSafety);

  CPPUNIT_TEST_SUITE_END();

private:
  cv::Mat image1,image2,image3,image4,image5;
  mitk::OpenCVToMitkImageFilter::Pointer testFilter;

public:

  void setUp() override
  {
    image1 = cv::imread(GetTestDataFilePath("NrrdWritingTestImage.jpg").c_str());
    image2 = cv::imread(GetTestDataFilePath("Png2D-bw.png").c_str());
    image3 = cv::imread(GetTestDataFilePath("OpenCV-Data/CroppedImage.png").c_str());
    image4 = cv::imread(GetTestDataFilePath("OpenCV-Data/GrabCutMask.png").c_str());
    image5 = cv::imread(GetTestDataFilePath("OpenCV-Data/GrabCutOutput.png").c_str());

    testFilter = mitk::OpenCVToMitkImageFilter::New();

    //change input
    testFilter->SetOpenCVMat(image1);
  }

  void tearDown() override
  {
  }

  void TestInitialization()
  {
    testFilter = mitk::OpenCVToMitkImageFilter::New();
    MITK_TEST_OUTPUT(<<"Testing Initialization");
  }

  void TestThreadSafety()
  {
    MITK_TEST_OUTPUT(<< "Testing Thread Safety with 2 Threads");

    //create two threads
    auto newThread1 = mitkTestOpenCVToMITKImageFilterThread::New();
    newThread1->setFilter(testFilter);
    newThread1->setImage(image1);
    auto newThread2 = mitkTestOpenCVToMITKImageFilterThread::New();
    newThread2->setFilter(testFilter);
    newThread2->setImage(image1);

    //start both
    newThread1->Start();
    newThread2->Start();

    int delay = 1;

    for (int i = 0; i < 100; i++)
    {
      //std::cout << "Run  " << i << std::endl;
      //wait a bit
      itksys::SystemTools::Delay(delay);

      //change input
      newThread1->setImage(image2);
      newThread1->setImage(image3);

      //wait a bit
      itksys::SystemTools::Delay(delay);

      //change input
      newThread1->setImage(image4);
      newThread1->setImage(image5);

      //wait a bit
      itksys::SystemTools::Delay(delay);

      //change input
      newThread1->setImage(image2);
      newThread1->setImage(image2);

      //wait a bit
      itksys::SystemTools::Delay(delay);

      //change input
      newThread1->setImage(image3);
      newThread1->setImage(image3);

      //wait a bit
      itksys::SystemTools::Delay(delay);
    }

    //stop both threads
    newThread1->Stop();
    newThread2->Stop();

    MITK_TEST_OUTPUT(<< "Testing Thread Safety with 2 Threads");

  }


private:

};
MITK_TEST_SUITE_REGISTRATION(mitkOpenCVToMitkImageFilter)
