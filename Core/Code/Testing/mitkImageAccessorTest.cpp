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


#include "mitkImage.h"
#include "mitkImageReadAccessor.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkImagePixelWriteAccessor.h"
#include "mitkImageWriteAccessor.h"
#include "mitkDataNodeFactory.h"
#include "mitkImageTimeSelector.h"
#include <itksys/SystemTools.hxx>
#include "itkBarrier.h"
#include <itkMultiThreader.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <mitkTestingMacros.h>

struct ThreadData
{
   itk::Barrier::Pointer m_Barrier;    // holds a pointer to the used barrier
   mitk::Image::Pointer data;              // some random data
   int m_NoOfThreads;                  // holds the number of generated threads
   bool m_Successful;   // to check if everything worked
};

ITK_THREAD_RETURN_TYPE ThreadMethod(void* data)
{
   /* extract data pointer from Thread Info structure */
   struct itk::MultiThreader::ThreadInfoStruct * pInfo =
      (struct itk::MultiThreader::ThreadInfoStruct*)data;

   // some data validity checking
   if (pInfo == NULL)
   {
      return ITK_THREAD_RETURN_VALUE;
   }
   if (pInfo->UserData == NULL)
   {
      return ITK_THREAD_RETURN_VALUE;
   }

   // obtain user data for processing
   ThreadData* threadData = (ThreadData*) pInfo->UserData;

   srand( pInfo->ThreadID );

   mitk::Image::Pointer im = threadData->data;


   int nrSlices = im->GetDimension(2);

   // Create randomly a PixelRead- or PixelWriteAccessor for a slice and access all pixels in it.
   try
   {
      if(rand() % 2)
      {
         mitk::ImageDataItem* iDi = im->GetSliceData(rand() % nrSlices);
         while(!iDi->IsComplete());

         //MITK_INFO << "pixeltype: " << im->GetPixelType().GetComponentTypeAsString();

         if ((im->GetPixelType().GetComponentTypeAsString() == "short") && (im->GetDimension() == 3) )
         {
            // Use pixeltype&dimension specific read accessor

            int xlength = im->GetDimension(0);
            int ylength = im->GetDimension(1);

            mitk::ImagePixelReadAccessor<short,2> readAccessor(im, iDi);

            itk::Index<2> idx;
            for(int i=0; i<xlength; ++i)
            {
               for(int j=0; j<ylength; ++j)
               {
                  idx[0] = i;
                  idx[1] = j;
                  short value = readAccessor.GetPixelByIndexSafe(idx);
                  value = 0; // this is only done to prevent compiler warning
               }
            }
         }
         else
         {
            // use general accessor
            mitk::ImageReadAccessor* iRA = new mitk::ImageReadAccessor(im,iDi);
            delete iRA;
         }


      }
      else
      {
         mitk::ImageDataItem* iDi = im->GetSliceData(rand() % nrSlices);
         while(!iDi->IsComplete());


         if ((im->GetPixelType().GetComponentTypeAsString() == "short") && (im->GetDimension() == 3) )
         {
            // Use pixeltype&dimension specific read accessor

            int xlength = im->GetDimension(0);
            int ylength = im->GetDimension(1);

            mitk::ImagePixelWriteAccessor<short,2> writeAccessor(im, iDi);

            itk::Index<2> idx;
            for(int i=0; i<xlength; ++i)
            {
               for(int j=0; j<ylength; ++j)
               {
                  idx[0] = i;
                  idx[1] = j;
                  short newVal = rand() % 16000;
                  writeAccessor.SetPixelByIndexSafe(idx, newVal);
                  short val = writeAccessor.GetPixelByIndexSafe(idx);
                  if (val != newVal)
                  {
                     threadData->m_Successful = false;
                  }
               }
            }
         }
         else
         {
            // use general accessor
            mitk::ImageWriteAccessor iB(im,iDi);
            void* pointer = iB.GetData();
            *((char*) pointer) = 0;
         }
      }
   }
   catch(mitk::MemoryIsLockedException e)
   {
      threadData->m_Successful = false;
      e.Print(std::cout);
   }
   catch(mitk::Exception e)
   {
      threadData->m_Successful = false;
      e.Print(std::cout);
   }





   // data processing end!
   threadData->m_Barrier->Wait();
   return ITK_THREAD_RETURN_VALUE;
}



int mitkImageAccessorTest(int argc, char* argv[])
{
   MITK_TEST_BEGIN("mitkImageAccessorTest");

   std::cout << "Loading file: ";
   if(argc==0)
   {
      std::cout<<"no file specified [FAILED]"<<std::endl;
      return EXIT_FAILURE;
   }
   mitk::Image::Pointer image = NULL;
   mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();
   try
   {
      factory->SetFileName( argv[1] );
      factory->Update();

      if(factory->GetNumberOfOutputs()<1)
      {
         std::cout<<"file could not be loaded [FAILED]"<<std::endl;
         return EXIT_FAILURE;
      }
      mitk::DataNode::Pointer node = factory->GetOutput( 0 );
      image = dynamic_cast<mitk::Image*>(node->GetData());
      if(image.IsNull())
      {
         std::cout<<"file not an image - test will not be applied [PASSED]"<<std::endl;
         std::cout<<"[TEST DONE]"<<std::endl;
         return EXIT_SUCCESS;
      }
   }
   catch ( itk::ExceptionObject & ex )
   {
      std::cout << "Exception: " << ex << "[FAILED]" << std::endl;
      return EXIT_FAILURE;
   }

   // CREATE THREADS

   image->GetGeometry()->Initialize();

   itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
   unsigned int noOfThreads = 1;

   // initialize barrier
   itk::Barrier::Pointer barrier = itk::Barrier::New();
   barrier->Initialize( noOfThreads + 1);               // add one for we stop the base thread when the worker threads are processing

   ThreadData* threadData = new ThreadData;
   threadData->m_Barrier = barrier;
   threadData->m_NoOfThreads = noOfThreads;
   threadData->data = image;
   threadData->m_Successful = true;

   // spawn threads
   for(unsigned int i=0; i < noOfThreads; ++i)
   {
      threader->SpawnThread(ThreadMethod, threadData);
   }
   // stop the base thread during worker thread execution
   barrier->Wait();

   // terminate threads
   for(unsigned int j=0; j < noOfThreads; ++j)
   {
      threader->TerminateThread(j);
   }

   bool TestSuccessful = threadData->m_Successful ;
   delete threadData;

   MITK_TEST_CONDITION_REQUIRED( TestSuccessful, "Testing image access from multiple threads");

   MITK_TEST_END();
}
