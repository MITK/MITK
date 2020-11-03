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
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

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
  mitkNewMacro1Param(mitkTestOpenCVToMITKImageFilterThread, itk::MultiThreader::Pointer);

  int NumberOfMessages;

protected:

  mitkTestOpenCVToMITKImageFilterThread(itk::MultiThreader::Pointer MultiThreader)
  {
    ThreadID = -1;
    NumberOfMessages = 0;
    m_MultiThreader = MultiThreader;

  }

  bool ThreadRunning;

  int ThreadID;

  cv::Mat currentImage;

  mitk::OpenCVToMitkImageFilter::Pointer m_testedFilter;

  itk::MultiThreader::Pointer m_MultiThreader;

  void DoSomething()
  {
    while (ThreadRunning)
    {
      m_testedFilter->SetOpenCVMat(currentImage);
      m_testedFilter->Update();
      mitk::Image::Pointer result;
      result = m_testedFilter->GetOutput();
      //std::cout << "Thread " << ThreadID << " Update Call" << std::endl;
    }
  }


  static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* pInfoStruct)
  {
    /* extract this pointer from Thread Info structure */
    struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
    if (pInfo == nullptr)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    if (pInfo->UserData == nullptr)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    mitkTestOpenCVToMITKImageFilterThread *thisthread = (mitkTestOpenCVToMITKImageFilterThread*)pInfo->UserData;

    if (thisthread != nullptr)
      thisthread->DoSomething();

    return ITK_THREAD_RETURN_VALUE;
  }

public:

  int Start()
  {
    ThreadRunning = true;
    this->ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);
    return ThreadID;
  }

  void Stop()
  {
    ThreadRunning = false;
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
    std::vector<unsigned int> threadIDs;
    std::vector<mitkTestOpenCVToMITKImageFilterThread::Pointer> threads;
    itk::MultiThreader::Pointer multiThreader = itk::MultiThreader::New();
    MITK_TEST_OUTPUT(<< "Testing Thread Safety with 2 Threads");

    //create two threads
    mitkTestOpenCVToMITKImageFilterThread::Pointer newThread1 = mitkTestOpenCVToMITKImageFilterThread::New(multiThreader);
    newThread1->setFilter(testFilter);
    newThread1->setImage(image1);
    threads.push_back(newThread1);
    mitkTestOpenCVToMITKImageFilterThread::Pointer newThread2 = mitkTestOpenCVToMITKImageFilterThread::New(multiThreader);
    newThread2->setFilter(testFilter);
    newThread2->setImage(image1);
    threads.push_back(newThread2);


    //start both
    unsigned int id1 = newThread1->Start();
    unsigned int id2 = newThread2->Start();

    int delay = 1;

    for (int i = 0; i < 10000; i++)
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

    multiThreader->TerminateThread(id1);
    multiThreader->TerminateThread(id2);

    MITK_TEST_OUTPUT(<< "Testing Thread Safety with 2 Threads");

  }


private:

};
MITK_TEST_SUITE_REGISTRATION(mitkOpenCVToMitkImageFilter)
