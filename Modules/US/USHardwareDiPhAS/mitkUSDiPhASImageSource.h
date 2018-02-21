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
#include <iostream>
#include <fstream>
#include <mitkOphirPyro.h>


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

  typedef itk::Image< float, 3 > itkFloatImageType;
  typedef mitk::USDiPhASDeviceCustomControls::DataType DataType;
  typedef mitk::USDiPhASDeviceCustomControls::SavingSettings SavingSettings;

  /**
    * Implementation of the superclass method. Returns the pointer
    * to the mitk::Image filled by DiPhAS API callback.
    */
  virtual void GetNextRawImage( std::vector<mitk::Image::Pointer>& ) override;

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
  void SetSavingSettings(SavingSettings settings);
  void SetVerticalSpacing(float mm);

  void ModifyDataType(DataType dataT);
  void ModifyUseBModeFilter(bool isSet);
  void ModifyScatteringCoefficient(int coeff);
  void ModifyCompensateForScattering(bool useIt);
  void ModifyEnergyCompensation(bool compensate);

  /**
  * Sets the spacing used in the image based on the informations of the ScanMode in USDiPhAS Device
  */
  void UpdateImageGeometry();

protected:
  void SetDataType(DataType dataT);
  void SetUseBModeFilter(bool isSet);

	USDiPhASImageSource(mitk::USDiPhASDevice* device);
  virtual ~USDiPhASImageSource( );

  /** This vector holds all the images we record, if recording is set to active. */
  std::vector<mitk::Image::Pointer>     m_RecordedImages;
  std::vector<mitk::Image::Pointer>     m_RawRecordedImages;
  std::vector<long long>                m_ImageTimestampRecord;
  std::vector<long long>                m_ImageTimestampBuffer;
  long long                             m_CurrentImageTimestamp;
  bool                                  m_CurrentlyRecording;
  mitk::OphirPyro::Pointer              m_Pyro;
  bool                                  m_PyroConnected;

  std::vector<Image::Pointer>           m_FluenceCompOriginal;
  std::vector<Image::Pointer>           m_FluenceCompResized;
  std::vector<itk::Image<float, 3>::Pointer>     m_FluenceCompResizedItk;

  std::vector<mitk::Image::Pointer>     m_ImageBuffer;
  int                                   m_LastWrittenImage;
  int                                   m_BufferSize;

  unsigned int                          m_ImageDimensions[3];
  mitk::Vector3D                        m_ImageSpacing;

  mitk::Image::Pointer ApplyBmodeFilter(mitk::Image::Pointer image, bool useLogFilter = false);
  mitk::Image::Pointer CutOffTop(mitk::Image::Pointer image, int cutOffSize = 165);
  mitk::Image::Pointer ResampleOutputVertical(mitk::Image::Pointer image, float verticalSpacing = 0.1);

  mitk::Image::Pointer ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scatteringCoefficient);
  mitk::Image::Pointer ApplyResampling(mitk::Image::Pointer inputImage, mitk::Vector3D outputSpacing, unsigned int outputSize[3]);
  mitk::Image::Pointer MultiplyImage(mitk::Image::Pointer inputImage, double value);

  void OrderImagesInterleaved(Image::Pointer PAImage, Image::Pointer USImage, std::vector<Image::Pointer> recordedList, bool raw);
  void OrderImagesUltrasound(Image::Pointer USImage, std::vector<Image::Pointer> recordedList);

  void GetPixelValues(itk::Index<3> pixel, std::vector<float>& values);
  float GetPixelValue(itk::Index<3> pixel);
  std::vector<float>                    m_PixelValues;

  mitk::USDiPhASDevice*                 m_Device;

  /** This is a callback to pass text data to the GUI. */
  std::function<void(QString)>          m_GUIOutput;

  /**
   * Variables for management of current state.
   */
  SavingSettings                  m_SavingSettings;

  float                           m_StartTime;
  bool                            m_UseGUIOutPut;

  BeamformerStateInfoNative       m_BeamformerInfos;
  bool                            m_UseBModeFilter;

  bool                            m_DataTypeModified;
  DataType                        m_DataTypeNext;

  bool                            m_UseBModeFilterModified;
  bool                            m_UseBModeFilterNext;

  float                           m_VerticalSpacing;
  float                           m_VerticalSpacingNext;
  bool                            m_VerticalSpacingModified;

  int                             m_ScatteringCoefficient;
  int                             m_ScatteringCoefficientNext;
  bool                            m_ScatteringCoefficientModified;

  bool                            m_CompensateForScattering;
  bool                            m_CompensateForScatteringNext;
  bool                            m_CompensateForScatteringModified;

  bool                            m_CompensateEnergy;
  bool                            m_CompensateEnergyNext;
  bool                            m_CompensateEnergyModified;

  DataType                        m_DataType;
};
} // namespace mitk

#endif // MITKUSDiPhASImageSource_H
