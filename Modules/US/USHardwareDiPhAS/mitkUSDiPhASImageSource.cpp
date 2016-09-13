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
  m_GUIOutput(nullptr)
{
}

mitk::USDiPhASImageSource::~USDiPhASImageSource( )
{
}

void mitk::USDiPhASImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  if ( image.IsNull() ) { image = mitk::Image::New(); }

  // write the data of m_Image to image
  if ( m_Image->IsInitialized() )
  {
    m_ImageMutex->Lock();

    // copy contents of the given image into the member variable
    image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetSliceData(0,0,0));
    image->SetSlice(inputReadAccessor.GetData());
    image->SetGeometry(m_Image->GetGeometry());

    m_ImageMutex->Unlock();
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
  if (imageData != nullptr && !m_Image.IsNull())
  {
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Lock(); }

    // initialize mitk::Image with given image size on the first time
    if ( ! m_Image->IsInitialized() )
    {
      unsigned int dim[]={imageWidth, imageHeight}; // image dimensions

      m_Image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 2, dim);

	  UpdateImageGeometry(); // update the image geometry
    }

    // lock the image for writing an copy the given buffer into the image then
    m_Image->SetSlice(imageData);
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Unlock(); }
  }
  if (m_GUIOutput) {
    m_GUIOutput->setText("hi");
  }
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

  MITK_DEBUG << "UpdateImageGeometry called!";
  MITK_DEBUG << "depth in mm: " << (recordTime*speedOfSound / 2);
  MITK_DEBUG << "new spacing: " << spacing;
}

void mitk::USDiPhASImageSource::setGUIOutput(QLabel* out)
{
  m_GUIOutput = out;
}