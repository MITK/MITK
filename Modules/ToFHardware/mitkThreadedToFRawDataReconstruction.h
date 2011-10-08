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
#ifndef __mitkThreadedToFRawDataReconstruction_h
#define __mitkThreadedToFRawDataReconstruction_h

// mitk includes
#include "mitkImageSource.h"
#include "mitkToFHardwareExports.h"

// itk includes
#include <itkBarrier.h>
#include <itkFastMutexLock.h>

// vtk includes
#include "vtkShortArray.h"

namespace mitk
{
  struct ThreadDataStruct 
  {
    std::vector<std::vector<short> > m_InputData;
    std::vector<float*> m_OutputData;
    unsigned int m_DataSize;
    unsigned int m_LineWidth;
    unsigned int m_FrameHeight;
    unsigned int m_ModulationFrequency;
    itk::Barrier::Pointer m_Barrier;              ///< barrier to synchronize ends of threads
    itk::FastMutexLock::Pointer m_ImageDataMutex; ///< mutex for coordinated access to image data
    itk::FastMutexLock::Pointer m_ThreadDataMutex; ///< mutex to control access to images
  };

  class MITK_TOFHARDWARE_EXPORT ThreadedToFRawDataReconstruction : public itk::ProcessObject
  {
   public: 

    mitkClassMacro( ThreadedToFRawDataReconstruction ,  itk::ProcessObject );
    itkNewMacro( Self );

    itkGetMacro(Init, bool);

    void SetChannelData(vtkShortArray* sourceData);

    void Initialize(int width, int height, int modulationFrequency, int sourceDataSize);

    void GetDistances(float* dist);
    void GetAmplitudes(float* ampl);
    void GetIntensities(float* inten);
    void GetAllData(float* dist, float* ampl, float* inten);

    void Update();

   protected:
    /*!
    \brief standard constructor
    */
    ThreadedToFRawDataReconstruction();
    /*!
    \brief standard destructor
    */
    ~ThreadedToFRawDataReconstruction();

    /*!
    \brief method generating the outputs of this filter. Called in the updated process of the pipeline.
    This method generates the two outputs of the ToFImageSource: The distance and the intensity image
    */
    virtual void GenerateData();
    
    /*!
    \brief method configures the camera output and prepares the thread data struct for threaded data
          generation
    */
    virtual void BeforeThreadedGenerateData();
 
    /*!
    \brief threader callback function for multi threaded data generation
    */
    static ITK_THREAD_RETURN_TYPE ThreadedGenerateDataCallbackFunction(void* data);

    // member variables
    int m_StackSize;                ///<
    int m_Width;
    int m_Height;
    int m_ImageSize;
    int m_SourceDataSize;
    vtkShortArray* m_SourceData;
    bool m_Init;

    float* m_CISDist;               ///< holds the distance information from for one distance image slice
    float* m_CISAmpl;               ///< holds the amplitude information from for one amplitude image slice
    float* m_CISInten;              ///< holds the intensity information from for one intensity image slice
    float* m_ThreadedCISDist;
    float* m_ThreadedCISAmpl; 
    float* m_ThreadedCISInten;
    
    itk::MultiThreader::Pointer m_Threader;
    ThreadDataStruct* m_ThreadData; 

  };
} //end mitk namespace

#endif // __mitkThreadedToFRawDataReconstruction_h
