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

#define _USE_MATH_DEFINES
#include <cmath>
#include "mitkPhotoacousticImage.h"
#include "itkBModeImageFilter.h"
#include "itkPhotoacousticBModeImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkPhotoacousticBeamformingFilter.h"
#include <chrono>
#include <mitkAutoCropImageFilter.h>
#include <mitkPhotoacousticBModeFilter.h>


// itk dependencies
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include <itkIndex.h>
#include "itkMultiplyImageFilter.h"
#include "itkBSplineInterpolateImageFunction.h"
#include <mitkImageToItk.h>

// needed itk image filters
#include "mitkITKImageImport.h"
#include "itkFFTShiftImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include <itkAddImageFilter.h>
#include "itkFFT1DComplexConjugateToRealImageFilter.h"
#include "itkFFT1DRealToComplexConjugateImageFilter.h"

mitk::PhotoacousticImage::PhotoacousticImage()
{
  MITK_INFO << "[PhotoacousticImage Debug] created that image";
}

mitk::PhotoacousticImage::~PhotoacousticImage()
{
  MITK_INFO << "[PhotoacousticImage Debug] destroyed that image";
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBmodeFilter(mitk::Image::Pointer inputImage, BModeMethod method, bool UseLogFilter, float resampleSpacing)
{
  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkFloatImageType;
  typedef itk::IdentityTransform<double, 3> TransformType;

  if (method == BModeMethod::Abs)
  {
    auto input = ApplyCropping(inputImage, 0, 0, 0, 0, 0, inputImage->GetDimension(2) - 1);
    PhotoacousticBModeFilter::Pointer filter = PhotoacousticBModeFilter::New();
    filter->SetParameters(UseLogFilter);
    filter->SetInput(input);
    filter->Update();

    auto out = filter->GetOutput();

    if(resampleSpacing == 0)
      return out;

    typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
    ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

    itkFloatImageType::Pointer itkImage;
    mitk::CastToItkImage(out, itkImage);
    itkFloatImageType::SpacingType outputSpacing;
    itkFloatImageType::SizeType inputSize = itkImage->GetLargestPossibleRegion().GetSize();
    itkFloatImageType::SizeType outputSize = inputSize;

    outputSpacing[0] = itkImage->GetSpacing()[0];
    outputSpacing[1] = resampleSpacing;
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
  else if (method == BModeMethod::ShapeDetection)
  {
    typedef itk::BModeImageFilter < itkFloatImageType, itkFloatImageType > BModeFilterType;
    BModeFilterType::Pointer bModeFilter = BModeFilterType::New();  // LogFilter

    typedef itk::PhotoacousticBModeImageFilter < itkFloatImageType, itkFloatImageType > PhotoacousticBModeImageFilter;
    PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New(); // No LogFilter

    typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
    ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

    itkFloatImageType::Pointer itkImage;

    mitk::CastToItkImage(inputImage, itkImage);

    itkFloatImageType::Pointer bmode;

    if (UseLogFilter)
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

    // resampleSpacing == 0 means: do no resampling
    if (resampleSpacing == 0)
    {
      return mitk::GrabItkImageMemory(bmode);
    }

    itkFloatImageType::SpacingType outputSpacing;
    itkFloatImageType::SizeType inputSize = itkImage->GetLargestPossibleRegion().GetSize();
    itkFloatImageType::SizeType outputSize = inputSize;

    outputSpacing[0] = itkImage->GetSpacing()[0];
    outputSpacing[1] = resampleSpacing;
    outputSpacing[2] = itkImage->GetSpacing()[2];

    outputSize[1] = inputSize[1] * itkImage->GetSpacing()[1] / outputSpacing[1];

    resampleImageFilter->SetInput(bmode);
    resampleImageFilter->SetSize(outputSize);
    resampleImageFilter->SetOutputSpacing(outputSpacing);
    resampleImageFilter->SetTransform(TransformType::New());

    resampleImageFilter->UpdateLargestPossibleRegion();
    return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
  }
  return nullptr;
}

/*mitk::Image::Pointer mitk::PhotoacousticImage::ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scattering)
{
  typedef itk::Image< float, 3 > itkFloatImageType;
  typedef itk::MultiplyImageFilter <itkFloatImageType, itkFloatImageType > MultiplyImageFilterType;

  itkFloatImageType::Pointer itkImage;
  mitk::CastToItkImage(inputImage, itkImage);

  MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
  multiplyFilter->SetInput1(itkImage);
  multiplyFilter->SetInput2(m_FluenceCompResizedItk.at(m_ScatteringCoefficient));

  return mitk::GrabItkImageMemory(multiplyFilter->GetOutput());
}*/

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyResampling(mitk::Image::Pointer inputImage, unsigned int outputSize[2])
{
  typedef itk::Image< float, 3 > itkFloatImageType;

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();
  typedef itk::LinearInterpolateImageFunction<itkFloatImageType, double> T_Interpolator;
  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);

  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;

  outputSizeItk[0] = outputSize[0];
  outputSizeItk[1] = outputSize[1];
  outputSizeItk[2] = inputSizeItk[2];

  outputSpacingItk[0] = itkImage->GetSpacing()[0] * (static_cast<double>(inputSizeItk[0]) / static_cast<double>(outputSizeItk[0]));
  outputSpacingItk[1] = itkImage->GetSpacing()[1] * (static_cast<double>(inputSizeItk[1]) / static_cast<double>(outputSizeItk[1]));
  outputSpacingItk[2] = itkImage->GetSpacing()[2];

  typedef itk::IdentityTransform<double, 3> TransformType;
  T_Interpolator::Pointer _pInterpolator = T_Interpolator::New();
  resampleImageFilter->SetInput(itkImage);
  resampleImageFilter->SetSize(outputSizeItk);
  resampleImageFilter->SetOutputSpacing(outputSpacingItk);
  resampleImageFilter->SetTransform(TransformType::New());
  resampleImageFilter->SetInterpolator(_pInterpolator);

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyCropping(mitk::Image::Pointer inputImage, int above, int below, int right, int left, int minSlice, int maxSlice)
{
  
  unsigned int inputDim[3] = { inputImage->GetDimension(0), inputImage->GetDimension(1), inputImage->GetDimension(2) };
  unsigned int outputDim[3] = { inputImage->GetDimension(0) - left - right, inputImage->GetDimension(1) - (unsigned int)above - (unsigned int)below, (unsigned int)maxSlice - (unsigned int)minSlice + 1 };
 
  void* inputData;
  float* outputData = new float[outputDim[0] * outputDim[1] * outputDim[2]];

  ImageReadAccessor acc(inputImage);
  inputData = const_cast<void*>(acc.GetData());
  
  // convert the data to float by default
  // as of now only those float, short, float are used at all... though it's easy to add other ones
  if (inputImage->GetPixelType().GetTypeAsString() == "scalar (float)" || inputImage->GetPixelType().GetTypeAsString() == " (float)")
  {
    // copy the data into the cropped image
    for (unsigned short sl = 0; sl < outputDim[2]; ++sl)
    {
      for (unsigned short l = 0; l < outputDim[0]; ++l)
      {
        for (unsigned short s = 0; s < outputDim[1]; ++s)
        {
          outputData[l + s*(unsigned short)outputDim[0] + sl*outputDim[0] * outputDim[1]] = (float)((float*)inputData)[(l + left) + (s + above)*(unsigned short)inputDim[0] + (sl + minSlice)*inputDim[0] * inputDim[1]];
        }
      }
    }
  }
  else if (inputImage->GetPixelType().GetTypeAsString() == "scalar (short)" || inputImage->GetPixelType().GetTypeAsString() == " (short)")
  {
    // copy the data unsigned shorto the cropped image
    for (unsigned short sl = 0; sl < outputDim[2]; ++sl)
    {
      for (unsigned short l = 0; l < outputDim[0]; ++l)
      {
        for (unsigned short s = 0; s < outputDim[1]; ++s)
        {
          outputData[l + s*(unsigned short)outputDim[0] + sl*outputDim[0] * outputDim[1]] = (float)((short*)inputData)[(l + left) + (s + above)*(unsigned short)inputDim[0] + (sl + minSlice)*inputDim[0] * inputDim[1]];
        }
      }
    }
  }
  else if (inputImage->GetPixelType().GetTypeAsString() == "scalar (double)" || inputImage->GetPixelType().GetTypeAsString() == " (double)")
  {
    // copy the data unsigned shorto the cropped image
    for (unsigned short sl = 0; sl < outputDim[2]; ++sl)
    {
      for (unsigned short l = 0; l < outputDim[0]; ++l)
      {
        for (unsigned short s = 0; s < outputDim[1]; ++s)
        {
          outputData[l + s*(unsigned short)outputDim[0] + sl*outputDim[0] * outputDim[1]] = (float)((double*)inputData)[(l + left) + (s + above)*(unsigned short)inputDim[0] + (sl + minSlice)*inputDim[0] * inputDim[1]];
        }
      }
    }
  }
  else
  {
    MITK_INFO << "Could not determine pixel type";
  }

  mitk::Image::Pointer output = mitk::Image::New();
  output->Initialize(mitk::MakeScalarPixelType<float>(), 3, outputDim);
  output->SetSpacing(inputImage->GetGeometry()->GetSpacing());
  output->SetImportVolume(outputData, 0, 0, mitk::Image::ReferenceMemory);

  return output;
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBeamforming(mitk::Image::Pointer inputImage, BeamformingFilter::beamformingSettings config, int cutoff, std::function<void(int, std::string)> progressHandle)
{
  // crop the image
  // set the Maximum Size of the image to 4096
  unsigned short lowerCutoff = 0;
  if (((unsigned int)(4096 + cutoff)) < inputImage->GetDimension(1))
  {
    lowerCutoff = (unsigned short)(inputImage->GetDimension(1) - 4096);
  }

  config.RecordTime = config.RecordTime - (cutoff + lowerCutoff) / inputImage->GetDimension(1) * config.RecordTime; // adjust the recorded time lost by cropping
  progressHandle(0, "cropping image");
  if (!config.partial)
  {
    config.CropBounds[0] = 0;
    config.CropBounds[1] = inputImage->GetDimension(2) - 1;
  }

  Image::Pointer processedImage = ApplyCropping(inputImage, cutoff, lowerCutoff, 0, 0, config.CropBounds[0], config.CropBounds[1]);

  // resample the image in horizontal direction
  if (inputImage->GetDimension(0) != config.ReconstructionLines)
  {
    progressHandle(0, "resampling image");
    auto begin = std::chrono::high_resolution_clock::now();
    unsigned int dim[2] = { config.ReconstructionLines, processedImage->GetDimension(1) };
    processedImage = ApplyResampling(processedImage, dim);
    auto end = std::chrono::high_resolution_clock::now();
    MITK_DEBUG << "Upsampling from " << inputImage->GetDimension(0) << " to " << config.ReconstructionLines << " lines completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
  }

  // perform the beamforming
  BeamformingFilter::Pointer Beamformer = BeamformingFilter::New();
  Beamformer->SetInput(processedImage);
  Beamformer->Configure(config);
  Beamformer->SetProgressHandle(progressHandle);
  Beamformer->UpdateLargestPossibleRegion();

  processedImage = Beamformer->GetOutput();

  return processedImage;
}

mitk::Image::Pointer mitk::PhotoacousticImage::BandpassFilter(mitk::Image::Pointer data, float recordTime, float BPHighPass, float BPLowPass, float alpha)
{
  bool powerOfTwo = false;
  int finalPower = 0;
  for (int i = 1; pow(2, i) <= data->GetDimension(1); ++i)
  {
    finalPower = i;
    if (pow(2, i) == data->GetDimension(1))
    {
      powerOfTwo = true;
    }
  }
  if (!powerOfTwo)
  {
    unsigned int dim[2] = { data->GetDimension(0), (unsigned int)pow(2,finalPower+1)};
    data = ApplyResampling(data, dim);
  }

  MITK_INFO << data->GetDimension(0);

  // do a fourier transform, multiply with an appropriate window for the filter, and transform back
  typedef float PixelType;
  typedef itk::Image< PixelType, 3 > RealImageType;
  RealImageType::Pointer image;

  mitk::CastToItkImage(data, image);

  typedef itk::FFT1DRealToComplexConjugateImageFilter<RealImageType> ForwardFFTFilterType;
  typedef ForwardFFTFilterType::OutputImageType ComplexImageType;
  ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();
  forwardFFTFilter->SetInput(image);
  forwardFFTFilter->SetDirection(1);
  try
  {
    forwardFFTFilter->UpdateOutputInformation();
  }
  catch (itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    MITK_WARN << "Bandpass could not be applied";
    return data;
  }

  float singleVoxel = 1 / (recordTime / data->GetDimension(1)) / 2 / 1000;
  float cutoffPixelHighPass = std::min(BPHighPass / singleVoxel, (float)data->GetDimension(1) / 2);
  float cutoffPixelLowPass = std::min(BPLowPass / singleVoxel, (float)data->GetDimension(1) / 2 - cutoffPixelHighPass);

  RealImageType::Pointer fftMultiplicator = BPFunction(data, cutoffPixelHighPass, cutoffPixelLowPass, alpha);

  typedef itk::MultiplyImageFilter< ComplexImageType,
    RealImageType,
    ComplexImageType >
    MultiplyFilterType;
  MultiplyFilterType::Pointer multiplyFilter = MultiplyFilterType::New();
  multiplyFilter->SetInput1(forwardFFTFilter->GetOutput());
  multiplyFilter->SetInput2(fftMultiplicator);

  /*itk::ComplexToModulusImageFilter<ComplexImageType, RealImageType>::Pointer toReal = itk::ComplexToModulusImageFilter<ComplexImageType, RealImageType>::New();
  toReal->SetInput(forwardFFTFilter->GetOutput());
  return GrabItkImageMemory(toReal->GetOutput());
  return GrabItkImageMemory(fftMultiplicator);   *///DEBUG

  typedef itk::FFT1DComplexConjugateToRealImageFilter< ComplexImageType, RealImageType > InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput(multiplyFilter->GetOutput());
  inverseFFTFilter->SetDirection(1);

  return GrabItkImageMemory(inverseFFTFilter->GetOutput());
}

itk::Image<float, 3U>::Pointer mitk::PhotoacousticImage::BPFunction(mitk::Image::Pointer reference, int cutoffFrequencyPixelHighPass, int cutoffFrequencyPixelLowPass, float alpha)
{
  float* imageData = new float[reference->GetDimension(0)*reference->GetDimension(1)];
  
  // tukey window
  float width = reference->GetDimension(1) / 2 - (float)cutoffFrequencyPixelHighPass - (float)cutoffFrequencyPixelLowPass;
  float center = (float)cutoffFrequencyPixelHighPass / 2 + width / 2;


  MITK_INFO << width << "width  " << center << "center  " << alpha;

  for (unsigned int n = 0; n < reference->GetDimension(1); ++n)
  {
      imageData[reference->GetDimension(0)*n] = 0;
  }

  for (int n = 0; n < width; ++n)
  {
    if (n <= (alpha*(width - 1)) / 2)
    {
      imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] = (1 + cos(M_PI*(2 * n / (alpha*(width - 1)) - 1))) / 2;
    }
    else if (n >= (width - 1)*(1 - alpha / 2) && n <= (width - 1))
    {
      imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] = (1 + cos(M_PI*(2 * n / (alpha*(width - 1)) + 1 - 2 / alpha))) / 2;
    }
    else
    {
      imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] = 1;
    }
  }

  // Butterworth-Filter
  /*
  // first, write the HighPass
  if (cutoffFrequencyPixelHighPass != reference->GetDimension(1) / 2)
  {
    for (int n = 0; n < reference->GetDimension(1) / 2; ++n)
    {
      imageData[reference->GetDimension(0)*n] = 1 / (1 + pow(
        (float)n / (float)(reference->GetDimension(1) / 2 - cutoffFrequencyPixelHighPass)
        , 2 * butterworthOrder));
    }
  }
  else
  {
    for (int n = 0; n < reference->GetDimension(1) / 2; ++n)
    {
      imageData[reference->GetDimension(0)*n] = 1;
    }
  }

  // now, the LowPass
  for (int n = 0; n < reference->GetDimension(1) / 2; ++n)
  {
    imageData[reference->GetDimension(0)*n] *= 1 / (1 + pow(
      (float)(reference->GetDimension(1) / 2 - 1 - n) / (float)(reference->GetDimension(1) / 2 - cutoffFrequencyPixelLowPass)
      , 2 * butterworthOrder));
  }
  */

  // mirror the first half of the image
  for (unsigned int n = reference->GetDimension(1) / 2; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0)*n] = imageData[(reference->GetDimension(1) - (n + 1)) * reference->GetDimension(0)];
  }
  

  // copy and paste to all lines
  for (unsigned int line = 1; line < reference->GetDimension(0); ++line)
  {
    for (unsigned int sample = 0; sample < reference->GetDimension(1); ++sample)
    {
      imageData[reference->GetDimension(0)*sample + line] = imageData[reference->GetDimension(0)*sample];
    }
  }

  typedef itk::Image< float, 3U >  ImageType;
  ImageType::RegionType region;
  ImageType::IndexType start;
  start.Fill(0);

  region.SetIndex(start);

  ImageType::SizeType size;
  size[0] = reference->GetDimension(0);
  size[1] = reference->GetDimension(1);
  size[2] = reference->GetDimension(2);

  region.SetSize(size);

  ImageType::SpacingType SpacingItk;
  SpacingItk[0] = reference->GetGeometry()->GetSpacing()[0];
  SpacingItk[1] = reference->GetGeometry()->GetSpacing()[1];
  SpacingItk[2] = reference->GetGeometry()->GetSpacing()[2];

  ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(itk::NumericTraits<float>::Zero);
  image->SetSpacing(SpacingItk);

  ImageType::IndexType pixelIndex;

  for (ImageType::IndexValueType slice = 0; slice < reference->GetDimension(2); ++slice)
  {
    for (ImageType::IndexValueType line = 0; line < reference->GetDimension(0); ++line)
    {
      for (ImageType::IndexValueType sample = 0; sample < reference->GetDimension(1); ++sample)
      {
        pixelIndex[0] = line;
        pixelIndex[1] = sample;
        pixelIndex[2] = slice;

        image->SetPixel(pixelIndex, imageData[line + sample*reference->GetDimension(0)]);
      }
    }
  }

  delete[] imageData;

  return image;
}


/*
mitk::CropFilter::CropFilter()
  : m_PixelCalculation(NULL)
{
  this->AddSourceFile("CropFilter.cl");

  this->m_FilterID = "CropFilter";
}

mitk::CropFilter::~CropFilter()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }
}

void mitk::CropFilter::Update()
{
  //Check if context & program available
  if (!this->Initialize())
  {
    us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
    OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

    // clean-up also the resources
    resources->InvalidateStorage();
    mitkThrow() << "Filter is not initialized. Cannot update.";
  }
  else {
    // Execute
    this->Execute();
  }
}

void mitk::CropFilter::Execute()
{
  cl_int clErr = 0;

  try
  {
    this->InitExec(this->m_PixelCalculation);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Catched exception while initializing filter: " << e.what();
    return;
  }

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  cl_context gpuContext = resources->GetContext();

  unsigned int size = m_OutputDim[0] * m_OutputDim[1] * m_OutputDim[2] * m_Images;

  cl_mem cl_input = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * size, m_Data, &clErr);
  CHECK_OCL_ERR(clErr);

  // set kernel arguments
  clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &cl_input);
  clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_ushort), &(this->m_Images));

  CHECK_OCL_ERR(clErr);

  // execute the filter on a 3D NDRange
  this->ExecuteKernel(m_PixelCalculation, 3);

  // signalize the GPU-side data changed
  m_Output->Modified(GPU_DATA);
}

us::Module *mitk::CropFilter::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::CropFilter::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if (OclFilter::Initialize())
  {
    this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckCrop", &clErr);
    buildErr |= CHECK_OCL_ERR(clErr);
  }
  return (OclFilter::IsInitialized() && buildErr);
}

void mitk::CropFilter::SetInput(mitk::Image::Pointer image)
{
  if (image->GetDimension() != 3)
  {
    mitkThrowException(mitk::Exception) << "Input for " << this->GetNameOfClass() <<
      " is not 3D. The filter only supports 3D. Please change your input.";
  }
  OclImageToImageFilter::SetInput(image);
}

mitk::Image::Pointer mitk::CropFilter::GetOutput()
{
  if (m_Output->IsModified(GPU_DATA))
  {
    void* pData = m_Output->TransferDataToCPU(m_CommandQue);

    const unsigned int dimension = 3;
    unsigned int* dimensions = m_OutputDim;

    const mitk::SlicedGeometry3D::Pointer p_slg = m_Input->GetMITKImage()->GetSlicedGeometry();

    MITK_DEBUG << "Creating new MITK Image.";

    m_Output->GetMITKImage()->Initialize(this->GetOutputType(), dimension, dimensions);
    m_Output->GetMITKImage()->SetSpacing(p_slg->GetSpacing());
    m_Output->GetMITKImage()->SetGeometry(m_Input->GetMITKImage()->GetGeometry());
    m_Output->GetMITKImage()->SetImportVolume(pData, 0, 0, mitk::Image::ReferenceMemory);
  }

  MITK_DEBUG << "Image Initialized.";

  return m_Output->GetMITKImage();
}

*/
