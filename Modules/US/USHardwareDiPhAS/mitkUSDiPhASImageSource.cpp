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

// std dependencies
#include <ctime>
#include <chrono>
#include <algorithm>

// mitk dependencies
#include "mitkUSDiPhASDevice.h"
#include "mitkUSDiPhASImageSource.h"
#include <mitkIOUtil.h>
#include "mitkUSDiPhASBModeImageFilter.h"
#include "ITKUltrasound/itkBModeImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"

// itk dependencies
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include <itkIndex.h>
#include "itkMultiplyImageFilter.h"

mitk::USDiPhASImageSource::USDiPhASImageSource(mitk::USDiPhASDevice* device)
  : m_Device(device),
  m_StartTime(((float)std::clock()) / CLOCKS_PER_SEC),
  m_UseGUIOutPut(false),
  m_DataType(DataType::Image_uChar),
  m_GUIOutput(nullptr),
  m_UseBModeFilter(false),
  m_CurrentlyRecording(false),
  m_DataTypeModified(true),
  m_DataTypeNext(DataType::Image_uChar),
  m_CurrentImageTimestamp(0),
  m_PyroConnected(false),
  m_ImageTimestampBuffer(),
  m_VerticalSpacing(0),
  m_UseBModeFilterModified(false),
  m_UseBModeFilterNext(false),
  m_ScatteringCoefficientModified(false),
  m_CompensateForScatteringModified(false),
  m_VerticalSpacingModified(false),
  m_ScatteringCoefficient(15),
  m_CompensateForScattering(false),
  m_CompensateEnergy(false),
  m_CompensateEnergyNext(false),
  m_CompensateEnergyModified(false)
{
  m_BufferSize = 100;
  m_ImageTimestampBuffer.insert(m_ImageTimestampBuffer.begin(), m_BufferSize, 0);
  m_LastWrittenImage = m_BufferSize - 1;
  m_ImageBuffer.insert(m_ImageBuffer.begin(), m_BufferSize, nullptr);

  us::ModuleResource resourceFile;
  std::string name;
  m_FluenceCompOriginal.insert(m_FluenceCompOriginal.begin(), 5, Image::New());
  for (int i = 5; i <= 25; ++i)
  {
    name = "c:\\HomogeneousScatteringAssumptions\\Scattering" + std::to_string(i) + ".nrrd";

    m_FluenceCompOriginal.push_back(mitk::IOUtil::Load<mitk::Image>(name));
  }

  m_FluenceCompResized.insert(m_FluenceCompResized.begin(), 26, Image::New());
  m_FluenceCompResizedItk.insert(m_FluenceCompResizedItk.begin(), 26, itk::Image<float,3>::New());
}

mitk::USDiPhASImageSource::~USDiPhASImageSource()
{
  // close the pyro
  MITK_INFO("Pyro Debug") << "StopDataAcquisition: " << m_Pyro->StopDataAcquisition();
  MITK_INFO("Pyro Debug") << "CloseConnection: " << m_Pyro->CloseConnection();
  m_PyroConnected = false;
  m_Pyro = nullptr;
}

void mitk::USDiPhASImageSource::GetNextRawImage(std::vector<mitk::Image::Pointer>& imageVector)
{
  // modify all settings that have been changed here, so we don't get multithreading issues
  if (m_DataTypeModified)
  {
    SetDataType(m_DataTypeNext);
    m_DataTypeModified = false;
    UpdateImageGeometry();
  }
  if (m_UseBModeFilterModified)
  {
    SetUseBModeFilter(m_UseBModeFilterNext);
    m_UseBModeFilterModified = false;
  }
  if (m_VerticalSpacingModified)
  { 
    m_VerticalSpacing = m_VerticalSpacingNext;
    m_VerticalSpacingModified = false;
  }
  if (m_ScatteringCoefficientModified)
  {
    m_ScatteringCoefficient = m_ScatteringCoefficientNext;
    m_ScatteringCoefficientModified = false;
  }
  if (m_CompensateForScatteringModified)
  {
    m_CompensateForScattering = m_CompensateForScatteringNext;
    m_CompensateForScatteringModified = false;
  }
  if (m_CompensateEnergyModified)
  {
    m_CompensateEnergy = m_CompensateEnergyNext;
    m_CompensateEnergyModified = false;
  }

  if (imageVector.size() != 2)
  {
    imageVector.resize(2);
  }

  // make sure image is nullptr
  mitk::Image::Pointer image = nullptr;
  float ImageEnergyValue = 0;

  for (int i = 100; i > 90 && ImageEnergyValue <= 0; --i)
  {
    if (m_ImageTimestampBuffer[(m_LastWrittenImage + i) % 100] != 0)
    {
      ImageEnergyValue = m_Pyro->GetClosestEnergyInmJ(m_ImageTimestampBuffer[(m_LastWrittenImage + i) % 100]);
      if (ImageEnergyValue > 0) {
        image = &(*m_ImageBuffer[(m_LastWrittenImage + i) % 100]);
      }
    }
  }
  // if we did not get any usable Energy value, compensate using this default value
  if (image == nullptr)
  {
    image = &(*m_ImageBuffer[m_LastWrittenImage]);
    ImageEnergyValue = 40;
    if (image == nullptr)
      return;
  }

  // do image processing before displaying it
  if (image.IsNotNull())
  {
    itkFloatImageType::Pointer itkImage;

    mitk::CastToItkImage(image, itkImage);
    image = mitk::GrabItkImageMemory(itkImage); //thereby using float images
    image = CutOffTop(image, 165);

    // now apply filters to the image, if the options have been selected.
    if ((m_CompensateForScattering || m_UseBModeFilter) && m_DataType == DataType::Beamformed_Short)
    {
      if (m_Device->GetScanMode().beamformingAlgorithm == Beamforming::PlaneWaveCompound) // this is for ultrasound only mode
      {
        if (m_UseBModeFilter)
        {
          image = ApplyBmodeFilter(image, true);
          if (m_VerticalSpacing)
            image = ResampleOutputVertical(image, m_VerticalSpacing);
        }
      }

      else
      {
        Image::Pointer imagePA = Image::New();
        unsigned int dim[] = { image->GetDimension(0),image->GetDimension(1),1};
        imagePA->Initialize(image->GetPixelType(), 3, dim);
        imagePA->SetGeometry(image->GetGeometry());

        Image::Pointer imageUS = Image::New();
        imageUS->Initialize(image->GetPixelType(), 3, dim);
        imageUS->SetGeometry(image->GetGeometry());

        ImageReadAccessor inputReadAccessorCopyPA(image, image->GetSliceData(0));
        imagePA->SetSlice(inputReadAccessorCopyPA.GetData(), 0);
        ImageReadAccessor inputReadAccessorCopyUS(image, image->GetSliceData(1));
        imageUS->SetSlice(inputReadAccessorCopyUS.GetData(), 0);

        // first, seperate  the PA image from the USImages

        // then, we compensate the PAImage using our ImageEnergyValue
        if(m_CompensateEnergy)
          imagePA = MultiplyImage(imagePA, 1/ImageEnergyValue); // TODO: add the correct prefactor here!!!!

        // now we apply the BModeFilter
        if (m_UseBModeFilter)
        {
          imageUS = ApplyBmodeFilter(imageUS, true);     // the US Images get a logarithmic filter
          imagePA = ApplyBmodeFilter(imagePA, false);
        }

        ImageReadAccessor inputReadAccessorPA(imagePA, imagePA->GetSliceData(0));
        image->SetSlice(inputReadAccessorPA.GetData(), 0);
        ImageReadAccessor inputReadAccessorUS(imageUS, imageUS->GetSliceData(0));
        image->SetSlice(inputReadAccessorUS.GetData(), 1);
        if (m_VerticalSpacing)
        {
          image = ResampleOutputVertical(image, m_VerticalSpacing);
        }

        // and finally the scattering corrections
        if (m_CompensateForScattering)
        {
          auto curResizeImage = m_FluenceCompResized.at(m_ScatteringCoefficient); // just for convenience

          // update the fluence reference images!
          bool doResampling = image->GetDimension(0) != curResizeImage->GetDimension(0) || image->GetDimension(1) != curResizeImage->GetDimension(1)
            || image->GetGeometry()->GetSpacing()[0] != curResizeImage->GetGeometry()->GetSpacing()[0] || image->GetGeometry()->GetSpacing()[1] != curResizeImage->GetGeometry()->GetSpacing()[1];
          if (doResampling)
          {
            curResizeImage = ApplyResampling(m_FluenceCompOriginal.at(m_ScatteringCoefficient), image->GetGeometry()->GetSpacing(), image->GetDimensions());

            double* rawOutputData = new double[image->GetDimension(0)*image->GetDimension(1)];
            double* rawScatteringData = (double*)curResizeImage->GetData();
            int sizeRawScatteringData = curResizeImage->GetDimension(0) * curResizeImage->GetDimension(1);
            int imageSize = image->GetDimension(0)*image->GetDimension(1);

            //everything above 1.5mm is still inside the transducer; therefore the fluence compensation image has to be positioned a little lower
            float upperCutoffmm = 1.5;
            int lowerBound = std::round(upperCutoffmm / image->GetGeometry()->GetSpacing()[1])*image->GetDimension(0);
            int upperBound = lowerBound + sizeRawScatteringData;

            for (int i = 0; i < lowerBound && i < imageSize; ++i)
            {
              rawOutputData[i] = 0; // everything than cannot be compensated shall be treated as garbage, here the upper 0.15mm
            }
            for (int i = lowerBound; i < upperBound && i < imageSize; ++i)
            {
              rawOutputData[i] = 1 / rawScatteringData[i-lowerBound];
            }
            for (int i = upperBound; i < imageSize; ++i)
            {
              rawOutputData[i] = 0; // everything than cannot be compensated shall be treated as garbage
            }


            unsigned int dim[] = { image->GetDimension(0), image->GetDimension(1), 1 };
            curResizeImage->Initialize(mitk::MakeScalarPixelType<double>(), 3, dim);
            curResizeImage->SetGeometry(image->GetGeometry());
            curResizeImage->SetSlice(rawOutputData,0);

            delete[] rawOutputData;

            mitk::CastToItkImage(curResizeImage, m_FluenceCompResizedItk.at(m_ScatteringCoefficient));
            m_FluenceCompResized.at(m_ScatteringCoefficient) = mitk::GrabItkImageMemory(m_FluenceCompResizedItk.at(m_ScatteringCoefficient));

            MITK_INFO << "Resized a fluence image.";
          }
          // actually apply the scattering compensation
          imagePA = ApplyScatteringCompensation(imagePA, m_ScatteringCoefficient);
          ImageReadAccessor inputReadAccessorPA(imagePA, imagePA->GetSliceData(0));
          image->SetSlice(inputReadAccessorPA.GetData(), 0);
        }
      }
    }

    //TODO: completely rewrite this mess

    imageVector[0] = Image::New();
    unsigned int dim[] = { image->GetDimension(0),image->GetDimension(1),1 };
    imageVector[0]->Initialize(image->GetPixelType(), 3, dim);
    imageVector[0]->SetGeometry(image->GetGeometry());

    imageVector[1] = Image::New();
    imageVector[1]->Initialize(image->GetPixelType(), 3, dim);
    imageVector[1]->SetGeometry(image->GetGeometry());

    ImageReadAccessor inputReadAccessorCopyPA(image, image->GetSliceData(0));
    imageVector[0]->SetSlice(inputReadAccessorCopyPA.GetData(), 0);
    ImageReadAccessor inputReadAccessorCopyUS(image, image->GetSliceData(1));
    imageVector[1]->SetSlice(inputReadAccessorCopyUS.GetData(), 0);
  }
}

mitk::Image::Pointer mitk::USDiPhASImageSource::ApplyBmodeFilter(mitk::Image::Pointer image, bool useLogFilter)
{
  // we use this seperate ApplyBmodeFilter Method for processing of two-dimensional images

  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage

  typedef itk::BModeImageFilter < itkFloatImageType, itkFloatImageType > BModeFilterType;
  BModeFilterType::Pointer bModeFilter = BModeFilterType::New();  // LogFilter

  typedef itk::PhotoacousticBModeImageFilter < itkFloatImageType, itkFloatImageType > PhotoacousticBModeImageFilter;
  PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New(); // No LogFilter

  itkFloatImageType::Pointer itkImage;
  itkFloatImageType::Pointer bmode;
  mitk::CastToItkImage(image, itkImage);

  if (useLogFilter)
  {
    bModeFilter->SetInput(itkImage);
    bModeFilter->SetDirection(1);
    bmode = bModeFilter->GetOutput();
  }
  else
  {
    photoacousticBModeFilter->SetInput(itkImage);
    photoacousticBModeFilter->SetDirection(1);
    bmode = photoacousticBModeFilter->GetOutput();
  }
  return mitk::GrabItkImageMemory(bmode);
}

mitk::Image::Pointer mitk::USDiPhASImageSource::CutOffTop(mitk::Image::Pointer image, int cutOffSize)
{
  typedef itk::CropImageFilter < itkFloatImageType, itkFloatImageType > CutImageFilter;
  itkFloatImageType::SizeType cropSize;
  itkFloatImageType::Pointer itkImage;
  mitk::CastToItkImage(image, itkImage);

  cropSize[0] = 0;
  if(itkImage->GetLargestPossibleRegion().GetSize()[1] == 2048)
    cropSize[1] = cutOffSize;
  else
    cropSize[1] = 0;
  cropSize[2] = 0;
  CutImageFilter::Pointer cutOffFilter = CutImageFilter::New();
  cutOffFilter->SetInput(itkImage);
  cutOffFilter->SetLowerBoundaryCropSize(cropSize);
  cutOffFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(cutOffFilter->GetOutput());
}

mitk::Image::Pointer mitk::USDiPhASImageSource::ResampleOutputVertical(mitk::Image::Pointer image, float verticalSpacing)
{
  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(image, itkImage);
  itkFloatImageType::SpacingType outputSpacing;
  itkFloatImageType::SizeType inputSize = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSize = inputSize;

  outputSpacing[0] = itkImage->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / static_cast<double>(outputSize[0]));
  outputSpacing[1] = verticalSpacing;
  outputSpacing[2] = itkImage->GetSpacing()[2];

  outputSize[1] = inputSize[1] * itkImage->GetSpacing()[1] / outputSpacing[1];

  typedef itk::IdentityTransform<double, 3> TransformType;
  resampleImageFilter->SetInput(itkImage);
  resampleImageFilter->SetSize(outputSize);
  resampleImageFilter->SetOutputSpacing(outputSpacing);
  resampleImageFilter->SetTransform(TransformType::New());

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

mitk::Image::Pointer mitk::USDiPhASImageSource::ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scattering)
{
  typedef itk::MultiplyImageFilter <itkFloatImageType, itkFloatImageType > MultiplyImageFilterType;

  itkFloatImageType::Pointer itkImage;
  mitk::CastToItkImage(inputImage, itkImage);

  MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
  multiplyFilter->SetInput1(itkImage);
  multiplyFilter->SetInput2(m_FluenceCompResizedItk.at(m_ScatteringCoefficient));

  return mitk::GrabItkImageMemory(multiplyFilter->GetOutput());
}

mitk::Image::Pointer mitk::USDiPhASImageSource::ApplyResampling(mitk::Image::Pointer inputImage, mitk::Vector3D outputSpacing, unsigned int outputSize[3])
{
  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);

  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;
  itkFloatImageType::SpacingType inputSpacing = itkImage->GetSpacing();

  outputSizeItk[0] = outputSize[0];
  outputSizeItk[1] = 10*(inputSpacing[1] * inputSizeItk[1]) / (outputSpacing[1]);
  outputSizeItk[2] = 1;

  outputSpacingItk[0] = 0.996 * inputSpacing[0] * (static_cast<double>(inputSizeItk[0]) / static_cast<double>(outputSizeItk[0])); // TODO: find out why the spacing is not correct, so we need that factor; ?!?!
  outputSpacingItk[1] = inputSpacing[1] * (static_cast<double>(inputSizeItk[1]) / static_cast<double>(outputSizeItk[1]));
  outputSpacingItk[2] = outputSpacing[2];

  typedef itk::IdentityTransform<double, 3> TransformType;
  resampleImageFilter->SetInput(itkImage);
  resampleImageFilter->SetSize(outputSizeItk);
  resampleImageFilter->SetOutputSpacing(outputSpacingItk);
  resampleImageFilter->SetTransform(TransformType::New());

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

mitk::Image::Pointer mitk::USDiPhASImageSource::MultiplyImage(mitk::Image::Pointer inputImage, double value)
{
  typedef itk::MultiplyImageFilter <itkFloatImageType, itkFloatImageType > MultiplyImageFilterType;

  itkFloatImageType::Pointer itkImage;
  mitk::CastToItkImage(inputImage, itkImage);

  MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
  multiplyFilter->SetInput1(itkImage);
  multiplyFilter->SetConstant(value);

  return mitk::GrabItkImageMemory(multiplyFilter->GetOutput());
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
  if (m_DataTypeModified)
    return;

  if (!m_PyroConnected)
  {
    m_Pyro = mitk::OphirPyro::New();
    MITK_INFO << "[Pyro Debug] OpenConnection: " << m_Pyro->OpenConnection();
    MITK_INFO << "[Pyro Debug] StartDataAcquisition: " << m_Pyro->StartDataAcquisition();
    m_PyroConnected = true;
  }

  bool writeImage = ((m_DataType == DataType::Image_uChar) && (imageData != nullptr)) || ((m_DataType == DataType::Beamformed_Short) && (rfDataArrayBeamformed != nullptr));
  if (writeImage)
  {
    //get the timestamp we might save later on
    m_CurrentImageTimestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // create a new image and initialize it
    mitk::Image::Pointer image = mitk::Image::New();

    switch (m_DataType)
    {
      case DataType::Image_uChar: {
        m_ImageDimensions[0] = imageWidth;
        m_ImageDimensions[1] = imageHeight;
        m_ImageDimensions[2] = imageSetsTotal;
        image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, m_ImageDimensions);
        break;
      }
      case DataType::Beamformed_Short: {
        m_ImageDimensions[0] = beamformedLines;
        m_ImageDimensions[1] = beamformedSamples;
        m_ImageDimensions[2] = beamformedTotalDatasets;
        image->Initialize(mitk::MakeScalarPixelType<short>(), 3, m_ImageDimensions);
        break;
      }
    }
    image->GetGeometry()->SetSpacing(m_ImageSpacing);
    image->GetGeometry()->Modified();

    // write the given buffer into the image
    switch (m_DataType)
    {
      case DataType::Image_uChar: {
        for (unsigned char i = 0; i < imageSetsTotal; i++) {
          image->SetSlice(&imageData[i*imageHeight*imageWidth], i);
        }
        break;
      }

      case DataType::Beamformed_Short: {
        short* flipme = new short[beamformedLines*beamformedSamples*beamformedTotalDatasets];
        int pixelsPerImage = beamformedLines*beamformedSamples;

        for (unsigned char currentSet = 0; currentSet < beamformedTotalDatasets; currentSet++)
        {
            for (unsigned short sample = 0; sample < beamformedSamples; sample++)
            {
              for (unsigned short line = 0; line < beamformedLines; line++)
              {
                flipme[sample*beamformedLines + line + pixelsPerImage*currentSet]
                  = rfDataArrayBeamformed[line*beamformedSamples + sample + pixelsPerImage*currentSet];
              }
            } // the beamformed pa image is flipped by 90 degrees; we need to flip it manually
        }
        
        for (unsigned char i = 0; i < beamformedTotalDatasets; i++) {
          image->SetSlice(&flipme[i*beamformedLines*beamformedSamples], i);
          // set every image to a different slice
        }

        delete[] flipme;
        break;
      }
    }

    if (m_SavingSettings.saveRaw && m_CurrentlyRecording && rfDataChannelData != nullptr)
    {
      unsigned int dim[3];
      dim[0] = channelDataChannelsPerDataset;
      dim[1] = channelDataSamplesPerChannel;
      dim[2] = 1;
      short offset = m_Device->GetScanMode().accumulation * 2048;

      short* noOffset = new short[channelDataChannelsPerDataset*channelDataSamplesPerChannel*channelDataTotalDatasets];
      for (unsigned char set = 0; set < 1; ++set)// channelDataTotalDatasets; ++set) // we ignore the raw US images for now
      {
        for (unsigned short sam = 0; sam < channelDataSamplesPerChannel; ++sam)
        {
          for (unsigned short chan = 0; chan < channelDataChannelsPerDataset; ++chan)
          {
            noOffset[set*channelDataSamplesPerChannel*channelDataChannelsPerDataset + sam * channelDataChannelsPerDataset + chan] =
              rfDataChannelData[set*channelDataSamplesPerChannel*channelDataChannelsPerDataset + sam * channelDataChannelsPerDataset + chan] - offset; // this offset in the raw Images is given by the API...
          }
        }
      }

      // save the raw images when recording
      for (unsigned char i = 0; i < 1; ++i)// channelDataTotalDatasets; ++i) // we ignore the raw US images for now
      {
        mitk::Image::Pointer rawImage = mitk::Image::New();
        rawImage->Initialize(mitk::MakeScalarPixelType<short>(), 3, dim);

        rawImage->SetSlice(&noOffset[i*channelDataChannelsPerDataset*channelDataSamplesPerChannel]);

        float& recordTime = m_Device->GetScanMode().receivePhaseLengthSeconds;
        int& speedOfSound = m_Device->GetScanMode().averageSpeedOfSound;

        mitk::Vector3D rawSpacing;
        rawSpacing[0] = m_Device->GetScanMode().transducerPitchMeter * 1000; // save in mm
        rawSpacing[1] = recordTime / channelDataSamplesPerChannel * 1000000;  // save in us
        rawSpacing[2] = 1;

        rawImage->GetGeometry()->SetSpacing(rawSpacing);
        rawImage->GetGeometry()->Modified();

        m_RawRecordedImages.push_back(rawImage);
      }

      delete[] noOffset;
    }

    itk::Index<3> pixel = { {
        (itk::Index<3>::IndexValueType)(image->GetDimension(0) / 2),
        (itk::Index<3>::IndexValueType)(22.0/532.0*m_Device->GetScanMode().reconstructionSamplesPerLine), 
        0 } }; //22/532*2048 = 84
    if (!m_Pyro->IsSyncDelaySet() &&(image->GetPixelValueByIndex(pixel) < -30)) // #MagicNumber
    {
      MITK_INFO << "Setting SyncDelay now";
      m_Pyro->SetSyncDelay(m_CurrentImageTimestamp);
    }

    m_ImageTimestampBuffer[(m_LastWrittenImage + 1) % m_BufferSize] = m_CurrentImageTimestamp;
    m_ImageBuffer[(m_LastWrittenImage + 1) % m_BufferSize] = image;
    m_LastWrittenImage = (m_LastWrittenImage + 1) % m_BufferSize;

    // if the user decides to start recording, we feed the vector the generated images
    if (m_CurrentlyRecording) {
      for (unsigned char index = 0; index < image->GetDimension(2); ++index)
      {
        if (image->IsSliceSet(index))
        {
          m_RecordedImages.push_back(Image::New());
          unsigned int dim[] = { image ->GetDimension(0), image->GetDimension(1), 1};
          m_RecordedImages.back()->Initialize(image->GetPixelType(), 3, dim);
          m_RecordedImages.back()->SetGeometry(image->GetGeometry());

          mitk::ImageReadAccessor inputReadAccessor(image, image->GetSliceData(index));
          m_RecordedImages.back()->SetSlice(inputReadAccessor.GetData(),0);
        }
      }
      m_ImageTimestampRecord.push_back(m_CurrentImageTimestamp);
      // save timestamps for each laser image!
    }
  }
}

void mitk::USDiPhASImageSource::UpdateImageGeometry()
{
  MITK_INFO << "Retreaving Image Geometry Information for Spacing...";
  float& recordTime        = m_Device->GetScanMode().receivePhaseLengthSeconds;
  int& speedOfSound        = m_Device->GetScanMode().averageSpeedOfSound;
  float& pitch             = m_Device->GetScanMode().reconstructedLinePitchMmOrAngleDegree;
  int& reconstructionLines = m_Device->GetScanMode().reconstructionLines;

  switch (m_DataType)
  {
    case DataType::Image_uChar : {
      int& imageWidth = m_Device->GetScanMode().imageWidth;
      int& imageHeight = m_Device->GetScanMode().imageHeight;
      m_ImageSpacing[0] = pitch * reconstructionLines / imageWidth;
      m_ImageSpacing[1] = recordTime * speedOfSound / 2 * 1000 / imageHeight;
      break;
    }
    case DataType::Beamformed_Short : {
      int& imageWidth = reconstructionLines;
      int& imageHeight = m_Device->GetScanMode().reconstructionSamplesPerLine;
      m_ImageSpacing[0] = pitch;
      m_ImageSpacing[1] = recordTime * speedOfSound / 2 * 1000 / imageHeight;
      break;
    }
  }
  m_ImageSpacing[2] = 1;

  MITK_INFO << "Retreaving Image Geometry Information for Spacing " << m_ImageSpacing[0] << " ... " << m_ImageSpacing[1] << " ... " << m_ImageSpacing[2] << " ...[DONE]";
}

void mitk::USDiPhASImageSource::ModifyDataType(DataType dataT)
{
  m_DataTypeModified = true;
  m_DataTypeNext = dataT;
}

void mitk::USDiPhASImageSource::ModifyUseBModeFilter(bool isSet)
{
  m_UseBModeFilterModified = true;
  m_UseBModeFilterNext = isSet;
}

void mitk::USDiPhASImageSource::ModifyScatteringCoefficient(int coeff)
{
  m_ScatteringCoefficientNext = coeff;
  m_ScatteringCoefficientModified = true;
}

void mitk::USDiPhASImageSource::ModifyCompensateForScattering(bool useIt)
{
  m_CompensateForScatteringNext = useIt;
  m_CompensateForScatteringModified = true;
}

void mitk::USDiPhASImageSource::ModifyEnergyCompensation(bool compensate)
{
  m_CompensateEnergyNext = compensate;
  m_CompensateEnergyModified = true;
}

void mitk::USDiPhASImageSource::SetDataType(DataType dataT)
{
  if (dataT != m_DataType)
  {
    m_DataType = dataT;
    MITK_INFO << "Setting new DataType..." << dataT;
    switch (m_DataType)
    {
    case DataType::Image_uChar :
      MITK_INFO << "height: " << m_Device->GetScanMode().imageHeight << " width: " << m_Device->GetScanMode().imageWidth;
      break;
    case DataType::Beamformed_Short :
      MITK_INFO << "samples: " << m_Device->GetScanMode().reconstructionSamplesPerLine << " lines: " << m_Device->GetScanMode().reconstructionLines;
      break;
    }
  }
}

void mitk::USDiPhASImageSource::SetGUIOutput(std::function<void(QString)> out)
{
  USDiPhASImageSource::m_GUIOutput = out;
  m_StartTime = ((float)std::clock()) / CLOCKS_PER_SEC; //wait till the callback is available again
  m_UseGUIOutPut = false;
}

void mitk::USDiPhASImageSource::SetUseBModeFilter(bool isSet)
{
  m_UseBModeFilter = isSet;
}

void mitk::USDiPhASImageSource::SetVerticalSpacing(float mm)
{
  m_VerticalSpacingNext = mm;
  m_VerticalSpacingModified = true;
}

void mitk::USDiPhASImageSource::SetSavingSettings(SavingSettings settings)
{
  m_SavingSettings = settings;
}

// this is just a little function to set the filenames below right
inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

void mitk::USDiPhASImageSource::SetRecordingStatus(bool record)
{
  // start the recording process
  if (record)
  {
    m_RecordedImages.clear();  
    m_RawRecordedImages.clear(); // we make sure there are no leftovers
    m_ImageTimestampRecord.clear(); // also for the timestamps
    m_PixelValues.clear(); // aaaand for the pixel values

    if (m_SavingSettings.saveRaw)
    {
      m_Device->GetScanMode().transferChannelData = true;
      m_Device->UpdateScanmode();
      // set the raw Data to be transfered
    }

    // tell the callback to start recording images
    m_CurrentlyRecording = true;
  }
  // save images, end recording, and clean up
  else
  {
    m_CurrentlyRecording = false;

    m_Device->GetScanMode().transferChannelData = false; // make sure raw Channel Data is not transferred anymore!
    m_Device->UpdateScanmode();

    // get the time and date, put them into a nice string and create a folder for the images
    time_t time = std::time(nullptr);
    time_t* timeptr = &time;
    std::string currentDate = std::ctime(timeptr);
    replaceAll(currentDate, ":", "-");
    currentDate.pop_back();
    //std::string MakeFolder = "mkdir \"c:/DiPhASImageData/" + currentDate + "\"";
    //system(MakeFolder.c_str());

    // initialize file paths and the images
    Image::Pointer PAImage = Image::New();
    Image::Pointer USImage = Image::New();
    std::string pathPA = "c:\\ImageData\\" + currentDate + "-" + "PAbeamformed" + ".nrrd";
    std::string pathUS = "c:\\ImageData\\" + currentDate + "-" + "USImages" + ".nrrd";
    std::string pathTS = "c:\\ImageData\\" + currentDate + "-" + "ts" + ".csv";
    std::string pathS  = "c:\\ImageData\\" + currentDate + "-" + "Settings" + ".txt";

    // idon't forget the raw Images (if chosen to be saved)
    Image::Pointer PAImageRaw = Image::New();
    Image::Pointer USImageRaw = Image::New();
    std::string pathPARaw = "c:\\ImageData\\" + currentDate + "-" + "PAraw" + ".nrrd";
    std::string pathUSRaw = "c:\\ImageData\\" + currentDate + "-" + "USImagesRaw" + ".nrrd";

    if (m_Device->GetScanMode().beamformingAlgorithm == (int)Beamforming::Interleaved_OA_US) // save a PAImage if we used interleaved mode
    {
      // first, save the data, so the pyro does not aquire more unneccessary timestamps
      m_Pyro->SaveData();

      // now order the images and save them
      // the beamformed ones...
      if (m_SavingSettings.saveBeamformed)
      {
        OrderImagesInterleaved(PAImage, USImage, m_RecordedImages, false);
        mitk::IOUtil::Save(USImage, pathUS);
        mitk::IOUtil::Save(PAImage, pathPA);
      }

      // ...and the raw images
      if (m_SavingSettings.saveRaw)
      {
        OrderImagesInterleaved(PAImageRaw, USImageRaw, m_RawRecordedImages, true);
        // mitk::IOUtil::Save(USImageRaw, pathUSRaw);
        mitk::IOUtil::Save(PAImageRaw, pathPARaw);
      }

      // read the pixelvalues of the enveloped images at this position

      itk::Index<3> pixel = { {
          (itk::Index<3>::IndexValueType)(m_RecordedImages.at(0)->GetDimension(0) / 2),
          (itk::Index<3>::IndexValueType)(22.0 / 532.0*m_Device->GetScanMode().reconstructionSamplesPerLine), 
          0 } }; //22/532*2048 = 84

      GetPixelValues(pixel, m_PixelValues); // write the Pixelvalues to m_PixelValues

      // save the timestamps!
      ofstream timestampFile;

      timestampFile.open(pathTS);
      timestampFile << ",timestamp,pixelvalue"; // write the header

      for (int index = 0; index < m_ImageTimestampRecord.size(); ++index)
      {
        timestampFile << "\n" << index << "," << m_ImageTimestampRecord.at(index) << "," << m_PixelValues.at(index);
      }
      timestampFile.close();

      //save the settings!

      ofstream settingsFile;

      settingsFile.open(pathS);
      auto& sM = m_Device->GetScanMode();

      settingsFile << "[General Parameters]\n";
      settingsFile << "Scan Depth [mm] = " << sM.receivePhaseLengthSeconds * sM.averageSpeedOfSound / 2 * 1000 << "\n";
      settingsFile << "Speed of Sound [m/s] = " << sM.averageSpeedOfSound << "\n";
      settingsFile << "Excitation Frequency [MHz] = " << sM.transducerFrequencyHz/1000000 << "\n";
      settingsFile << "Voltage [V] = " << sM.voltageV << "\n";
      settingsFile << "TGC min = " << (int)sM.tgcdB[0] << " max = " << (int)sM.tgcdB[7] << "\n";

      settingsFile << "[Beamforming Parameters]\n";
      settingsFile << "Reconstructed Lines = " << sM.reconstructionLines << "\n";
      settingsFile << "Samples per Line = " << sM.reconstructionSamplesPerLine << "\n";

      settingsFile.close();
    }
    else if (m_Device->GetScanMode().beamformingAlgorithm == (int)Beamforming::PlaneWaveCompound) // save no PAImage if we used US only mode
    {
      OrderImagesUltrasound(USImage, m_RecordedImages);
      mitk::IOUtil::Save(USImage, pathUS);
    }

    m_PixelValues.clear();            
    m_RawRecordedImages.clear();      // clean up the pixel values
    m_RecordedImages.clear();         // clean up the images
    m_ImageTimestampRecord.clear();   // clean up the timestamps
  }
}

void mitk::USDiPhASImageSource::GetPixelValues(itk::Index<3> pixel, std::vector<float>& values)
{
  unsigned int events = 2;
  for (int index = 0; index < m_RecordedImages.size(); index += events)  // omit sound images
  {
    Image::Pointer image = m_RecordedImages.at(index);
    image = ApplyBmodeFilter(image);
    values.push_back(image.GetPointer()->GetPixelValueByIndex(pixel));
  }
}

void mitk::USDiPhASImageSource::OrderImagesInterleaved(Image::Pointer PAImage, Image::Pointer USImage, std::vector<Image::Pointer> recordedList, bool raw)
{
  unsigned int width  = 32;
  unsigned int height = 32;
  unsigned int events = m_Device->GetScanMode().transmitEventsCount + 1; // the PA event is not included in the transmitEvents, so we add 1 here
  if (!raw)
    events = 2; // the beamformed image array contains only the resulting image of multiple events

  if (raw)
  {
    width = recordedList.at(0)->GetDimension(0);
    height = recordedList.at(0)->GetDimension(1);
  }
  else if (m_DataType == DataType::Beamformed_Short)
  {
    width = m_Device->GetScanMode().reconstructionLines;
    height = m_Device->GetScanMode().reconstructionSamplesPerLine;
  }
  else if (m_DataType == DataType::Image_uChar)
  {
    width = m_Device->GetScanMode().imageWidth;
    height = m_Device->GetScanMode().imageHeight;
  }

  unsigned int dimLaser[] = { (unsigned int)width, (unsigned int)height, (unsigned int)(recordedList.size() / events)};
  unsigned int dimSound[] = { (unsigned int)width, (unsigned int)height, (unsigned int)(recordedList.size() / events * (events-1))};

  PAImage->Initialize(recordedList.back()->GetPixelType(), 3, dimLaser);
  PAImage->SetGeometry(recordedList.back()->GetGeometry());
  USImage->Initialize(recordedList.back()->GetPixelType(), 3, dimSound);
  USImage->SetGeometry(recordedList.back()->GetGeometry());

  for (int index = 0; index < recordedList.size(); ++index)
  {
    mitk::ImageReadAccessor inputReadAccessor(recordedList.at(index));

    if (index % events == 0)
    {
      PAImage->SetSlice(inputReadAccessor.GetData(), index / events);
    }
    else
    {
      if(!raw)
        USImage->SetSlice(inputReadAccessor.GetData(), ((index - (index % events)) / events) + (index % events)-1);
    }
  }
}

void mitk::USDiPhASImageSource::OrderImagesUltrasound(Image::Pointer USImage, std::vector<Image::Pointer> recordedList)
{
  unsigned int width  = 32;
  unsigned int height = 32;
  unsigned int events = m_Device->GetScanMode().transmitEventsCount;

  if (m_DataType == DataType::Beamformed_Short)
  {
    width = (unsigned int)m_Device->GetScanMode().reconstructionLines;
    height = (unsigned int)m_Device->GetScanMode().reconstructionSamplesPerLine;
  }
  else if (m_DataType == DataType::Image_uChar)
  {
    width = (unsigned int)m_Device->GetScanMode().imageWidth;
    height = (unsigned int)m_Device->GetScanMode().imageHeight;
  }

  unsigned int dimSound[] = { (unsigned int)width, (unsigned int)height, (unsigned int)recordedList.size()};

  USImage->Initialize(recordedList.back()->GetPixelType(), 3, dimSound);
  USImage->SetGeometry(recordedList.back()->GetGeometry());

  for (int index = 0; index < recordedList.size(); ++index)
  {
    mitk::ImageReadAccessor inputReadAccessor(recordedList.at(index));
    USImage->SetSlice(inputReadAccessor.GetData(), index);
  }
}