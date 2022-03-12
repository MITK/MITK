/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIOUtil.h"
#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkImagePixelWriteAccessor.h"
#include "mitkImageReadAccessor.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageWriteAccessor.h"
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkTestingMacros.h>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <random>

struct ThreadData
{
  ThreadData()
    : Successful(true),
      RandGen(std::random_device()())
  {
  }

  mitk::Image::Pointer Data;     // some random data
  bool Successful;               // to check if everything worked
  std::mt19937 RandGen;
  std::mutex RandGenMutex;
};

std::mutex testMutex;

itk::ITK_THREAD_RETURN_TYPE ThreadMethod(ThreadData *threadData)
{
  if (nullptr == threadData)
    return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;

  mitk::Image::Pointer im = threadData->Data;

  int nrSlices = im->GetDimension(2);

  std::uniform_int_distribution<> distrib(1, 1000000);

  // Create randomly a PixelRead- or PixelWriteAccessor for a slice and access all pixels in it.
  try
  {
    threadData->RandGenMutex.lock();
    auto even = distrib(threadData->RandGen) % 2;
    auto slice = distrib(threadData->RandGen) % nrSlices;
    threadData->RandGenMutex.unlock();

    if (even)
    {
      testMutex.lock();
      mitk::ImageDataItem *iDi = im->GetSliceData(slice);
      testMutex.unlock();
      while (!iDi->IsComplete())
      {
      }

      // MITK_INFO << "pixeltype: " << im->GetPixelType().GetComponentTypeAsString();

      if ((im->GetPixelType().GetComponentTypeAsString() == "short") && (im->GetDimension() == 3))
      {
        // Use pixeltype&dimension specific read accessor

        int xlength = im->GetDimension(0);
        int ylength = im->GetDimension(1);

        mitk::ImagePixelReadAccessor<short, 2> readAccessor(im, iDi);

        itk::Index<2> idx;
        for (int i = 0; i < xlength; ++i)
        {
          for (int j = 0; j < ylength; ++j)
          {
            idx[0] = i;
            idx[1] = j;
            readAccessor.GetPixelByIndexSafe(idx);
          }
        }
      }
      else
      {
        // use general accessor
        mitk::ImageReadAccessor *iRA = new mitk::ImageReadAccessor(im, iDi);
        delete iRA;
      }
    }
    else
    {
      testMutex.lock();
      mitk::ImageDataItem *iDi = im->GetSliceData(slice);
      testMutex.unlock();
      while (!iDi->IsComplete())
      {
      }

      if ((im->GetPixelType().GetComponentTypeAsString() == "short") && (im->GetDimension() == 3))
      {
        // Use pixeltype&dimension specific read accessor

        int xlength = im->GetDimension(0);
        int ylength = im->GetDimension(1);

        mitk::ImagePixelWriteAccessor<short, 2> writeAccessor(im, iDi);

        itk::Index<2> idx;
        for (int i = 0; i < xlength; ++i)
        {
          for (int j = 0; j < ylength; ++j)
          {
            idx[0] = i;
            idx[1] = j;
            threadData->RandGenMutex.lock();
            short newVal = distrib(threadData->RandGen) % 16000;
            threadData->RandGenMutex.unlock();
            writeAccessor.SetPixelByIndexSafe(idx, newVal);
            short val = writeAccessor.GetPixelByIndexSafe(idx);
            if (val != newVal)
            {
              threadData->Successful = false;
            }
          }
        }
      }
      else
      {
        // use general accessor
        mitk::ImageWriteAccessor iB(im, iDi);
        void *pointer = iB.GetData();
        *((char *)pointer) = 0;
      }
    }
  }
  catch ( const mitk::MemoryIsLockedException &e )
  {
    threadData->Successful = false;
    e.Print(std::cout);
  }
  catch ( const mitk::Exception &e )
  {
    threadData->Successful = false;
    e.Print(std::cout);
  }

  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

int mitkImageAccessorTest(int argc, char *argv[])
{
  MITK_TEST_BEGIN("mitkImageAccessorTest");

  std::cout << "Loading file: ";
  if (argc == 0)
  {
    std::cout << "no file specified [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  mitk::Image::Pointer image = nullptr;
  try
  {
    image = mitk::IOUtil::Load<mitk::Image>(std::string(argv[1]));

    if (image.IsNull())
    {
      MITK_TEST_FAILED_MSG(<< "file could not be loaded [FAILED]")
    }
  }
  catch ( const itk::ExceptionObject &ex )
  {
    MITK_TEST_FAILED_MSG(<< "Exception: " << ex.GetDescription() << "[FAILED]")
  }

  // CHECK INAPPROPRIATE AND SPECIAL USAGE

  // recursive mutex lock
  MITK_TEST_OUTPUT(<< "Testing a recursive mutex lock attempt, should end in an exception ...");

  MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::Exception)
  mitk::ImageWriteAccessor first(image);
  mitk::ImageReadAccessor second(image);
  MITK_TEST_FOR_EXCEPTION_END(mitk::Exception)

  // ignore lock mechanism in read accessor
  try
  {
    mitk::ImageWriteAccessor first(image);
    mitk::ImageReadAccessor second(image, nullptr, mitk::ImageAccessorBase::IgnoreLock);
    MITK_TEST_CONDITION_REQUIRED(true, "Testing the option flag \"IgnoreLock\" in ReadAccessor");
  }
  catch (const mitk::Exception & /*e*/)
  {
    MITK_TEST_CONDITION_REQUIRED(false, "Ignoring the lock mechanism leads to exception.");
  }

  // CREATE THREADS

  image->GetGeometry()->Initialize();

  ThreadData threadData;
  threadData.Data = image;

  constexpr size_t noOfThreads = 100; 
  std::array<std::thread, noOfThreads> threads;

  // spawn threads
  for (size_t i = 0; i < noOfThreads; ++i)
    threads[i] = std::thread(ThreadMethod, &threadData);

  // terminate threads
  for (size_t i = 0; i < noOfThreads; ++i)
  {
    if (threads[i].joinable())
      threads[i].join();
  }

  bool TestSuccessful = threadData.Successful;

  MITK_TEST_CONDITION_REQUIRED(TestSuccessful, "Testing image access from multiple threads");

  MITK_TEST_END();
}
