/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFImageRecorder_h
#define __mitkToFImageRecorder_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFPicImageWriter.h"
#include "mitkToFImageCsvWriter.h"
#include "mitkToFNrrdImageWriter.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkFastMutexLock.h"
#include "itkCommand.h"

namespace mitk
{
  /**
  * @brief Recorder class for ToF images
  *
  * This class represents a recorder for ToF data. A ToFCameraDevice is used to acquire the data. The acquired images
  * are then added to a ToFImageWriter that performs the actual writing.
  *
  * Recording can be performed either frame-based or continuously
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFImageRecorder : public itk::Object
  {
  public: 

    ToFImageRecorder();

    ~ToFImageRecorder();

    mitkClassMacro( ToFImageRecorder , itk::Object );

    itkNewMacro( Self );

    itkGetMacro( DistanceImageFileName, std::string );
    itkGetMacro( AmplitudeImageFileName, std::string );
    itkGetMacro( IntensityImageFileName, std::string );
    itkGetMacro( CaptureWidth, int );
    itkGetMacro( CaptureHeight, int );
    itkGetMacro( DistanceImageSelected, bool );
    itkGetMacro( AmplitudeImageSelected, bool );
    itkGetMacro( IntensityImageSelected, bool );
    itkGetMacro( NumOfFrames, int );
    itkGetMacro( FileFormat, std::string );

    itkSetMacro( DistanceImageFileName, std::string );
    itkSetMacro( AmplitudeImageFileName, std::string );
    itkSetMacro( IntensityImageFileName, std::string );
    itkSetMacro( DistanceImageSelected, bool );
    itkSetMacro( AmplitudeImageSelected, bool );
    itkSetMacro( IntensityImageSelected, bool );
    itkSetMacro( NumOfFrames, int );
    itkSetMacro( FileFormat, std::string );

    enum RecordMode{ PerFrames, Infinite };
    /*!
    \brief Returns the currently set RecordMode
    \return record mode: PerFrames ("Record specified number of frames"), Infinite ("Record until abort is required")
    */
    ToFImageRecorder::RecordMode GetRecordMode();
    /*!
    \brief Set the RecordMode
    \param recordMode: PerFrames ("Record specified number of frames"), Infinite ("Record until abort is required")
    */
    void SetRecordMode(ToFImageRecorder::RecordMode recordMode);
    /*!
    \brief Set the device used for acquiring ToF images
    \param aToFCameraDevice ToF camera device used.
    */
    void SetCameraDevice(ToFCameraDevice* aToFCameraDevice);
    /*!
    \brief Get the device used for acquiring ToF images
    \return ToF camera device used.
    */
    ToFCameraDevice* GetCameraDevice();
    /*!
    \brief Get the type of image to be recorded
    \return ToF image type: ToFImageType3D (0) or ToFImageType2DPlusT (1)
    */
    ToFImageWriter::ToFImageType GetToFImageType();
    /*!
    \brief Set the type of image to be recorded
    \param toFImageType type of the ToF image: ToFImageType3D (0) or ToFImageType2DPlusT (1)
    */
    void SetToFImageType(ToFImageWriter::ToFImageType toFImageType);
    /*!
    \brief Starts the recording by spawning a Thread which streams the data to a file. Aborting of the record process is controlled by the m_Abort flag
    */
    void StartRecording();
    /*!
    \brief Stops the recording by setting the m_Abort flag to false
    */
    void StopRecording();

  protected:

    /*!
    \brief Thread method acquiring data via the ToFCameraDevice and recording it to file via the ToFImageWriter
    */
    static ITK_THREAD_RETURN_TYPE RecordData(void* pInfoStruct);

    // data acquisition
    ToFCameraDevice::Pointer m_ToFCameraDevice; ///< ToFCameraDevice used for acquiring the images
    int m_CaptureWidth; ///< width (x-dimension) of the images to record.
    int m_CaptureHeight; ///< height (y-dimension) of the images to record.
    int m_PixelNumber; ///< number of pixels (widht*height) of the images to record
    int m_SourceDataSize; ///< size of the source data provided by the device
    int m_ImageSequence; ///< number of images currently acquired
    float* m_IntensityArray; ///< array holding the intensity data
    float* m_DistanceArray; ///< array holding the distance data
    float* m_AmplitudeArray; ///< array holding the amplitude data
    char* m_SourceDataArray; ///< array holding the source data

    // data writing
    ToFImageWriter::Pointer m_ToFImageWriter; ///< image writer writing the acquired images to a file

    std::string m_DistanceImageFileName; ///< file name for saving the distance image
    std::string m_AmplitudeImageFileName; ///< file name for saving the amplitude image
    std::string m_IntensityImageFileName; ///< file name for saving the intensity image
    
    int m_NumOfFrames; ///< number of frames to be recorded by this recorder
    ToFImageWriter::ToFImageType m_ToFImageType; ///< type of image to be recorded: ToFImageType3D (0) or ToFImageType2DPlusT (1)
    ToFImageRecorder::RecordMode m_RecordMode; ///< mode of recording the images: specified number of frames (PerFrames) or infinite (Infinite)
    std::string m_FileFormat; ///< file format for saving images. If .csv is chosen, ToFImageCsvWriter is used

    bool m_DistanceImageSelected; ///< flag indicating if distance image should be recorded
    bool m_AmplitudeImageSelected; ///< flag indicating if amplitude image should be recorded
    bool m_IntensityImageSelected; ///< flag indicating if intensity image should be recorded

    // threading
    itk::MultiThreader::Pointer m_MultiThreader; ///< member for thread-handling (ITK-based)
    int m_ThreadID; ///< ID of the thread recording the data
    itk::FastMutexLock::Pointer m_AbortMutex; ///< mutex for thread-safe data access of abort flag
    bool m_Abort; ///< flag controlling the abort mechanism of the recording procedure. For thread-safety only use in combination with m_AbortMutex
    
  private:

  };
} //END mitk namespace
#endif
