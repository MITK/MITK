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

#ifndef MITKUSDiPhASImageSource_H_HEADER_INCLUDED_
#define MITKUSDiPhASImageSource_H_HEADER_INCLUDED_


#include "mitkUSImageSource.h"
#include "mitkUSDiPhASCustomControls.h"

#include "Framework.IBMT.US.CWrapper.h"

#include "mitkImageReadAccessor.h"
#include "itkFastMutexLock.h"
#include <functional>
#include <qstring.h>
#include <ctime>
#include <string>
#include <mutex>


namespace mitk {

class USDiPhASDevice;
/**
  * \brief Implementation of mitk::USImageSource for DiPhAS API devices.
  * The method mitk::USImageSource::GetNextRawImage() is implemented for
  * getting images from the DiPhAS API.
  *
  * The image data is given to this class from the DiPhAS API by calling 
  * a callback method that writes the image data to an mitk::image
  */
class USDiPhASImageSource : public USImageSource
{
  
public:
  mitkClassMacro(USDiPhASImageSource, USImageSource);
  mitkNewMacro1Param(Self, mitk::USDiPhASDevice*);
  itkCloneMacro(Self);

  typedef mitk::USDiPhASDeviceCustomControls::DataType DataType;

  /**
    * Implementation of the superclass method. Returns the pointer
    * to the mitk::Image filled by DiPhAS API callback.
    */
  virtual void GetNextRawImage( mitk::Image::Pointer& );

  /**
    * The API calls this function to pass the image data to the
	  * user; here the m_Image is updated
    */
  void mitk::USDiPhASImageSource::ImageDataCallback(
    short* rfDataChannelData,
    int& channelDataChannelsPerDataset,
    int& channelDataSamplesPerChannel,
    int& channelDataTotalDatasets,

    short* rfDataArrayBeamformed,
    int& beamformedLines,
    int& beamformedSamples,
    int& beamformedTotalDatasets,

    unsigned char* imageData,
    int& imageWidth,
    int& imageHeight,
    int& imagePixelFormat,
    int& imageSetsTotal,

    double& timeStamp);

  void SetGUIOutput(std::function<void(QString)> out);

  /** This starts or ends the recording session*/
  void SetRecordingStatus(bool record);

  void ModifyDataType(DataType DataT);
  void ModifyUseBModeFilter(bool isSet);

  /**
  * Sets the spacing used in the image based on the informations of the ScanMode in USDiPhAS Device
  */
  void UpdateImageGeometry();

protected:
  void SetDataType(DataType DataT);
  void SetUseBModeFilter(bool isSet);

	USDiPhASImageSource(mitk::USDiPhASDevice* device);
  virtual ~USDiPhASImageSource( );

  /** This vector holds all the images we record, if recording is set to active. */
  std::vector<mitk::Image::Pointer>     m_recordedImages;

  mitk::Image::Pointer ApplyBmodeFilter(mitk::Image::Pointer inputImage);
  mitk::Image::Pointer ApplyBmodeFilter2d(mitk::Image::Pointer inputImage);

  void OrderImagesInterleaved(Image::Pointer LaserImage, Image::Pointer SoundImage);
  void OrderImagesUltrasound(Image::Pointer SoundImage);

  /** Reinitializes the image according to the DataType set. */
  void UpdateImageDataType(int imageHeight, int imageWidth);

  /** This image holds the image to be displayed right now*/
  mitk::Image::Pointer                  m_Image;

  mitk::USDiPhASDevice*                 m_device;

  /** This is a callback to pass text data to the GUI. */
  std::function<void(QString)>          m_GUIOutput;


  /**
   * Variables for management of current state.
   */
  float                           startTime;
  bool                            useGUIOutPut;
  BeamformerStateInfoNative       BeamformerInfos;
  bool                            useBModeFilter;
  bool                            currentlyRecording;
  bool                            m_DataTypeModified;
  DataType                        m_DataTypeNext;
  bool                            m_UseBModeFilterModified;
  bool                            m_UseBModeFilterNext;

  DataType                        m_DataType;

  std::mutex m_ImageMutex;
};
} // namespace mitk

#endif // MITKUSDiPhASImageSource_H
