/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 11415 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// mitk includes
#include "mitkThreadedToFRawDataReconstruction.h"
#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"

// stl includes
#include <iostream>
#include <string>
#include <algorithm>

// vtk includes
#include <vtkLookupTable.h>
#include <vtkFieldData.h>
#include <vtkSmartPointer.h>

// itk includes
#include <itkMultiThreader.h>
#include <itksys/SystemTools.hxx>

#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <new>

#define _USE_MATH_DEFINES
#include <math.h>

#define PI M_PI;
#define cAir 299704944;
#define fMod 20000000;

namespace mitk
{

  ThreadedToFRawDataReconstruction::ThreadedToFRawDataReconstruction(): m_Threader(0),
    m_CISDist(0), m_CISAmpl(0), m_CISInten(0), 
    m_ThreadedCISDist(0), m_ThreadedCISAmpl(0), m_ThreadedCISInten(0), 
    m_Init(0), m_Width(0), m_Height(0), m_SourceDataSize(0), m_ImageSize(0), m_SourceData(0)
  {
    m_ThreadData = new ThreadDataStruct;
    m_ThreadData->m_ModulationFrequency = fMod;

    m_ThreadData->m_ImageDataMutex = itk::FastMutexLock::New();
    m_ThreadData->m_ThreadDataMutex = itk::FastMutexLock::New();

    m_StackSize = 1;
  }

  ThreadedToFRawDataReconstruction::~ThreadedToFRawDataReconstruction()
  {
    if(m_ThreadData != NULL)
      delete m_ThreadData;

    if(m_CISDist != NULL)
      delete[] m_CISDist;

    if(m_CISAmpl != NULL)
      delete[] m_CISAmpl;

    if(m_CISInten != NULL)
      delete[] m_CISInten;

    if(m_ThreadedCISInten != NULL)
      delete[] m_ThreadedCISInten;

    if(m_ThreadedCISAmpl != NULL)
      delete[] m_ThreadedCISAmpl;

    if(m_ThreadedCISDist != NULL)
      delete[] m_ThreadedCISDist;

  }

void ThreadedToFRawDataReconstruction::Initialize(int width, int height, int modulationFrequency, int sourceDataSize )
{
  m_Width = width;
  m_Height = height;
  m_SourceDataSize = sourceDataSize;
  m_ImageSize = width * height;
  m_ThreadData->m_ModulationFrequency = modulationFrequency * 1e6;

  if(!m_Init)
  {
    m_SourceData = vtkShortArray::New();
    m_SourceData->SetNumberOfComponents(m_SourceDataSize);
    m_SourceData->SetNumberOfTuples(4);
    m_SourceData->Allocate(1);

    m_CISDist = new float[m_ImageSize];
    m_CISAmpl = new float[m_ImageSize];
    m_CISInten = new float[m_ImageSize];
    m_ThreadedCISDist = new float[m_ImageSize];
    m_ThreadedCISAmpl = new float[m_ImageSize];
    m_ThreadedCISInten = new float[m_ImageSize];

    m_ThreadData->m_OutputData.push_back( m_ThreadedCISDist );
    m_ThreadData->m_OutputData.push_back( m_ThreadedCISAmpl );
    m_ThreadData->m_OutputData.push_back( m_ThreadedCISInten );

    m_Init = true;
  }
}

void ThreadedToFRawDataReconstruction::SetChannelData(vtkShortArray* sourceData)
{ 
  m_SourceData->DeepCopy(sourceData);
}

void ThreadedToFRawDataReconstruction::GetDistances(float* dist)
{
  memcpy(dist, m_CISDist, m_ImageSize*sizeof(float) );
}

void ThreadedToFRawDataReconstruction::GetAmplitudes(float* ampl)
{  
  memcpy(ampl, m_CISAmpl, m_ImageSize*sizeof(float));
}

void ThreadedToFRawDataReconstruction::GetIntensities(float* inten)
{
  memcpy(inten, m_CISInten, m_ImageSize*sizeof(float));  
}

void ThreadedToFRawDataReconstruction::GetAllData(float* dist, float* ampl, float* inten)
{
  memcpy(dist, m_CISDist, m_ImageSize*sizeof(float) );
  memcpy(ampl, m_CISAmpl, m_ImageSize*sizeof(float));
  memcpy(inten, m_CISInten, m_ImageSize*sizeof(float));
}

  void ThreadedToFRawDataReconstruction::GenerateData()
  {
    if(m_Init)
    {
      this->BeforeThreadedGenerateData();
    }
  }

  void ThreadedToFRawDataReconstruction::BeforeThreadedGenerateData()
  {
    int sourceDataSize = m_SourceDataSize;
    int lineWidth = m_Width;
    int frameHeight = m_Height;
    int channelSize = lineWidth*frameHeight << 1;
    int quadChannelSize = channelSize * 0.25;

    std::vector<short> quad = std::vector<short>(quadChannelSize);
    
    // clean the thread data array
    m_ThreadData->m_InputData.erase(m_ThreadData->m_InputData.begin(),m_ThreadData->m_InputData.end());

    int channelNo = 0;
    while(channelNo < m_SourceData->GetNumberOfTuples())
    {
      short* sourceData = new short[channelSize];
      m_SourceData->GetTupleValue(channelNo, sourceData);
      quad.insert(quad.begin(), sourceData,  sourceData+channelSize);
      m_ThreadData->m_InputData.push_back(quad);
      delete[]sourceData;
      ++channelNo;
    }

    if(m_Threader.IsNull())
    {
      m_Threader = this->GetMultiThreader();
    }
    int maxThreadNr = 0;

    if(m_Threader->GetGlobalDefaultNumberOfThreads()> 5)
    {
      maxThreadNr = 5;
    }
    else if(m_Threader->GetGlobalMaximumNumberOfThreads()>5)
    {
      maxThreadNr = 5;
    }
    else
    {
      maxThreadNr = m_Threader->GetGlobalMaximumNumberOfThreads();
    }

    if ( m_ThreadData->m_Barrier.IsNull())
    {
      m_ThreadData->m_Barrier = itk::Barrier::New();
      m_ThreadData->m_Barrier->Initialize(maxThreadNr);     // 
    }
    m_ThreadData->m_DataSize = quadChannelSize;
    m_ThreadData->m_LineWidth = lineWidth;
    m_ThreadData->m_FrameHeight = frameHeight * 0.25;
    std::vector<int> threadIDVector;
    int threadcounter = 0;
    while(threadcounter != maxThreadNr-1)
    {
      if (m_Threader->GetNumberOfThreads() < m_Threader->GetGlobalMaximumNumberOfThreads())
      {
        int threadID = m_Threader->SpawnThread(this->ThreadedGenerateDataCallbackFunction, m_ThreadData);
        threadIDVector.push_back(threadID);
        threadcounter++;
      }
    }
    m_ThreadData->m_Barrier->Wait();

    int count = 0;
    while(count != threadIDVector.size())
    {
      m_Threader->TerminateThread(threadIDVector.at(count));
      count++;
    }
    m_ThreadData->m_ImageDataMutex->Lock();
    memcpy(m_CISDist, m_ThreadData->m_OutputData.at(0), (channelSize * 0.5)*sizeof(float));
    memcpy(m_CISAmpl, m_ThreadData->m_OutputData.at(1), (channelSize * 0.5)*sizeof(float));
    memcpy(m_CISInten, m_ThreadData->m_OutputData.at(2), (channelSize * 0.5)*sizeof(float));
    m_ThreadData->m_ImageDataMutex->Unlock();

  }

  ITK_THREAD_RETURN_TYPE  ThreadedToFRawDataReconstruction::ThreadedGenerateDataCallbackFunction(void* data)
  {
    /* extract this pointer from Thread Info structure */
    struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)data;
    if (pInfo == NULL)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    if (pInfo->UserData == NULL)
    {
      return ITK_THREAD_RETURN_VALUE;
    }
    int quadrant = pInfo->ThreadID;
    ThreadDataStruct* threadData = (ThreadDataStruct*) pInfo->UserData;

    // some needed variables
    int x = 0;
    double phi = 0;
    double phi2 = 0;
    double A1 = 0;
    double A2 = 0;
    double A3 = 0;
    double A4 = 0;
    double A5 = 0;
    double A6 = 0;
    double A7 = 0;
    double A8 = 0;
    double A3m1 = 0;
    double A4m2 = 0;
    double A7m5 = 0;
    double A8m6 = 0;
    double cair = cAir;
    double pi = PI;
    double twoPi = pi + pi;
    long modFreq = fMod;

    threadData->m_ThreadDataMutex->Lock();
    std::vector<short>  quad1 = threadData->m_InputData.at(0);
    std::vector<short>  quad2 = threadData->m_InputData.at(1);
    std::vector<short>  quad3 = threadData->m_InputData.at(2);
    std::vector<short>  quad4 = threadData->m_InputData.at(3);
    int index = quadrant << 1;
    int index2 = 3-quadrant;
    modFreq = threadData->m_ModulationFrequency;
    int linewidth = threadData->m_LineWidth;
    int frameheight = threadData->m_FrameHeight;
    threadData->m_ThreadDataMutex->Unlock();

    double intermed1 = cair/(pi*(modFreq << 2));
    double intermed2 = intermed1*500;
    int doubleLwidth = linewidth << 1;
    int datasize = doubleLwidth*frameheight << 2;


    do 
    {
      index += doubleLwidth;
      x++;
      do 
      {
        index -= 8;
        A1 = htons(quad1.at(index));
        A2 = htons(quad2.at(index));
        A3 = htons(quad3.at(index));
        A4 = htons(quad4.at(index));
        A5 = htons(quad1.at(index+1));
        A6 = htons(quad2.at(index+1));
        A7 = htons(quad3.at(index+1));
        A8 = htons(quad4.at(index+1));

        phi  = atan2((A3 - A1),(A2 - A4)) + pi; 
        phi2 = atan2((A7 - A5),(A6 - A8)); 
        if(phi2<0) phi2 +=twoPi;

        A3m1 = A3*A3 - 2*A3*A1 + A1*A1;
        A4m2 = A4*A4 - 2*A4*A2 + A2*A2;
        A7m5 = A7*A7 - 2*A7*A5 + A5*A5;
        A8m6 = A8*A8 - 2*A8*A6 + A6*A6;
        threadData->m_ImageDataMutex->Lock();
        threadData->m_OutputData.at(0)[index2] = (phi+phi2)*intermed2;                        //(((phi*intermed1) + (phi2*intermed1))/2)*1000;  //   
        threadData->m_OutputData.at(1)[index2] = (sqrt(A3m1 + A4m2)+sqrt(A7m5 + A8m6))*0.5;   //(sqrt(A3m1 + A4m2)/2) + (sqrt(A7m5 + A8m6)/2);
        threadData->m_OutputData.at(2)[index2] = (A1+A2+A3+A4+A5+A6+A7+A8)*0.125;
        threadData->m_ImageDataMutex->Unlock();

        index2 += 4;
      }while(index2 <= (x*linewidth) - (1+quadrant));

      index += doubleLwidth;

    }while(index < datasize);

    threadData->m_Barrier->Wait();

    return ITK_THREAD_RETURN_VALUE;
  }

  void ThreadedToFRawDataReconstruction::Update()
  {
    this->GenerateData();
  }
} // end mitk namespace
