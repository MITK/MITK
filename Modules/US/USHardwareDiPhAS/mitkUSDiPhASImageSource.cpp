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

#include "mitkUSDiPhASImageSource.h"
#include "mitkUSDiPhASDevice.h"


mitk::USDiPhASImageSource::USDiPhASImageSource(mitk::USDiPhASDevice* device)
	: m_Image(mitk::Image::New()),
	m_ImageMutex(itk::FastMutexLock::New()),
  m_device(device),
  startTime(((float)std::clock()) / CLOCKS_PER_SEC),
  useGUIOutPut(false),
  DataType(0),
  displayedEvent(0),
  m_GUIOutput(nullptr)
{
}

mitk::USDiPhASImageSource::~USDiPhASImageSource( )
{
}

#include "mitkUSDiPhASBModeImageFilter.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"

void mitk::USDiPhASImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  m_ImageMutex->Lock();

  if (m_Image->IsInitialized())
  {
    //initialize the image the first time
    if (image.IsNull()) {
      image = mitk::Image::New();
      image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    }

    // write the data of m_Image to image if it changed
    if (image->GetPixelType() != m_Image->GetPixelType() ||
      image->GetDimensions() != m_Image->GetDimensions()) {
      image = mitk::Image::New();
      image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    }

    // copy contents of the given image into the member variable, slice after slice
    bool isSet = true;
    int sliceNumber = 0;
    while(isSet) {
      if (isSet = m_Image->IsSliceSet(sliceNumber)) {
        mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetSliceData(sliceNumber, 0, 0));
        image->SetSlice(inputReadAccessor.GetData(), sliceNumber);

        typedef itk::Image< float, 3 > itkFloatImageType;
        typedef itk::Image< unsigned char, 3 > itkUcharImageType;
        typedef itk::PhotoacousticBModeImageFilter < itkFloatImageType, itkUcharImageType > PhotoacousticBModeImageFilter;

        PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New();
        itkFloatImageType::Pointer itkImage;
        mitk::CastToItkImage(m_Image, itkImage);
        photoacousticBModeFilter->SetInput(itkImage);
        photoacousticBModeFilter->SetDirection(0);
        image = mitk::GrabItkImageMemory(photoacousticBModeFilter->GetOutput());
        /**
         *this is just a Test to set up everything for the BModeFilterImplementation
         */
      }
      else {
        break;
      }
      ++sliceNumber;
    }
    image->SetGeometry(m_Image->GetGeometry());
  }

  m_ImageMutex->Unlock();

  if (!useGUIOutPut && m_GUIOutput) {
    // Need to do this because the program initializes the GUI twice
    // this is probably a bug in UltrasoundSupport, if it's fixed the timing becomes unneccesary
    float timePassed = ((float)std::clock()) / CLOCKS_PER_SEC - startTime;
    if (timePassed > 10)
    {
      useGUIOutPut = true;
    }
  }
  if (useGUIOutPut) {
    // pass some beamformer state infos to the GUI
    getSystemInfo(&BeamformerInfos);

    std::ostringstream s;
    s << "state info: PRF:" << BeamformerInfos.systemPRF << "Hz, datarate: " << BeamformerInfos.dataTransferRateMBit << "MBit/s";

    m_GUIOutput(QString::fromStdString(s.str()));
  }
}

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
    int& imageBytesPerPixel,
    int& imageSetsTotal,

    double& timeStamp)
{
  bool writeImage = ((DataType == 0) && (imageData != nullptr)) || ((DataType == 1) && (rfDataArrayBeamformed != nullptr));
  if (!m_Image.IsNull() && writeImage)
  {
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Lock(); }

    // initialize mitk::Image with given image size on the first time
    if ( ! m_Image->IsInitialized() )
    {
      UpdateImageDataType(imageHeight, imageWidth);     // update data type and image pixel dimensions
    }

    // lock the image for writing an copy the given buffer into the image then
    switch (DataType)
    {
      case 0: {
        for (int i = 0; i < imageSetsTotal; i++) {
          m_Image->SetSlice(&imageData[i*imageHeight*imageWidth], i);
        }
        break;
      }
      case 1: {
        short* flipme = new short[beamformedLines*beamformedSamples*beamformedTotalDatasets];
        int pixelsPerImage = beamformedLines*beamformedSamples;

        for (char currentSet = 0; currentSet < beamformedTotalDatasets; currentSet++)
        {
          for (unsigned int sample = 0; sample < beamformedSamples; sample++)
          {
            for (short line = 0; line < beamformedLines; line++)
            {
              flipme[sample*beamformedLines + line + pixelsPerImage*currentSet]
                = rfDataArrayBeamformed[line*beamformedSamples + sample + pixelsPerImage*currentSet];
            }
          } // the beamformed image is flipped by 90 degrees; we need to flip it manually
        }

        for (int i = 0; i < beamformedTotalDatasets; i++) {
          m_Image->SetSlice(&flipme[i*beamformedLines*beamformedSamples], i);
          // set every image to a different slice
        }
        delete flipme;
        break;
      }
    }
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Unlock(); }
  }
}

void mitk::USDiPhASImageSource::UpdateImageDataType(int imageHeight, int imageWidth)
{
  unsigned int dim[] = { imageWidth, imageHeight, displayedEvent+1 }; // image dimensions; every image needs a seperate slice!
  m_ImageMutex->Lock();

  m_Image = mitk::Image::New();

  switch (DataType)
  {
    case 0: {
      m_Image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dim);
      break;
    }
    case 1: {
      m_Image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dim);
      break;
    }
  } // 0:imageData 1:beamformed

  m_ImageMutex->Unlock();
  UpdateImageGeometry();                            // update the image geometry

  startTime = ((float)std::clock()) / CLOCKS_PER_SEC; //wait till the callback is available again
  useGUIOutPut = false;
}

void mitk::USDiPhASImageSource::UpdateImageGeometry()
{
  mitk::Vector3D spacing;
  float& recordTime        = m_device->GetScanMode().receivePhaseLengthSeconds;
  int& speedOfSound        = m_device->GetScanMode().averageSpeedOfSound;
  int& imageWidth          = m_device->GetScanMode().imageWidth;
  int& imageHeight         = m_device->GetScanMode().imageHeight;
  float& pitch             = m_device->GetScanMode().reconstructedLinePitchMmOrAngleDegree;
  int& reconstructionLines = m_device->GetScanMode().reconstructionLines;

  spacing[0] = (pitch*reconstructionLines)/imageWidth;
  spacing[1] = ((recordTime*speedOfSound/2)*1000)/imageHeight;
  spacing[2] = 1; 
  //recalculate correct spacing

  m_ImageMutex->Lock();
  if (m_Image.IsNotNull() && (m_Image->GetGeometry() != NULL))
  {
	  m_Image->GetGeometry()->SetSpacing(spacing);
	  m_Image->GetGeometry()->Modified();
  }
  else
  {
	  MITK_WARN << "image or geometry was NULL, can't adapt geometry";
  }
  m_ImageMutex->Unlock();

  MITK_INFO << "UpdateImageGeometry called!";
  MITK_INFO << "depth in mm: " << (recordTime*speedOfSound / 2);
  MITK_INFO << "new spacing: " << spacing;
}

void mitk::USDiPhASImageSource::setDataType(int DataT)
{
  if (DataT != DataType)
  {
    DataType = DataT;
    UpdateImageDataType(m_device->GetScanMode().imageHeight, m_device->GetScanMode().imageWidth);
  }
}

void mitk::USDiPhASImageSource::SetDisplayedEvent(int event)
{
  displayedEvent = event;
  auto& ScanMode = m_device->GetScanMode();
  UpdateImageDataType(ScanMode.imageHeight, ScanMode.imageWidth);
}

void mitk::USDiPhASImageSource::setGUIOutput(std::function<void(QString)> out)
{
  USDiPhASImageSource::m_GUIOutput = out;
}