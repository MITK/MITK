/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTransformationOperation.h>

#include <mutex>

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <itkImage.h>
#include <itkRecursiveMultiResolutionPyramidImageFilter.h>
#include <itkLaplacianRecursiveGaussianImageFilter.h>

// Wavelet
#include <itkWaveletFrequencyForward.h>
#include <itkWaveletFrequencyFilterBankGenerator.h>
#include <itkHeldIsotropicWavelet.h>
#include <itkVowIsotropicWavelet.h>
#include <itkSimoncelliIsotropicWavelet.h>
#include <itkShannonIsotropicWavelet.h>
#include <itkForwardFFTImageFilter.h>
#include <itkInverseFFTImageFilter.h>
#include <itkFFTPadImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkVnlFFTImageFilterInitFactory.h>
#include <itkZeroFluxNeumannBoundaryCondition.h>
#include <itkPeriodicBoundaryCondition.h>
#include <itkConstantBoundaryCondition.h>
//#include <itkComplexToRealImageFilter.h>
#include <itkCastImageFilter.h>

#include <itkUnaryFunctorImageFilter.h>
#include <mitkImageMappingHelper.h>
#include <mitkMAPAlgorithmHelper.h>
#include <itkImageDuplicator.h>

namespace mitk
{
  namespace Functor
  {
    template< class TInput>
    class ThresholdValue
    {
    public:
      ThresholdValue() {};
      ~ThresholdValue() {};
      bool operator!=(const ThresholdValue &) const
      {
        return false;
      }
      bool operator==(const ThresholdValue & other) const
      {
        return !(*this != other);
      }
      inline unsigned short operator()(const TInput & A) const
      {
        if (A < value)
          return 0;
        else
          return 1;
      }
      double value = 0.0;
    };

    template< class TInput, class TOutput>
    class RoundValue
    {
    public:
      RoundValue() {};
      ~RoundValue() {};
      bool operator!=(const RoundValue &) const
      {
        return false;
      }
      bool operator==(const RoundValue & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A) const
      {
        return std::round(A);
      }
    };
  }
}

template<typename TPixel, unsigned int VImageDimension>
static void ExecuteMultiResolution(itk::Image<TPixel, VImageDimension>* image, unsigned int numberOfLevels, bool outputAsDouble, std::vector<mitk::Image::Pointer> &resultImages)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> DoubleOutputType;
  typedef itk::RecursiveMultiResolutionPyramidImageFilter<ImageType, ImageType> ImageTypeFilterType;
  typedef itk::RecursiveMultiResolutionPyramidImageFilter<ImageType, DoubleOutputType> DoubleTypeFilterType;

  if (outputAsDouble)
  {
    typename DoubleTypeFilterType::Pointer recursiveMultiResolutionPyramidImageFilter = DoubleTypeFilterType::New();
    recursiveMultiResolutionPyramidImageFilter->SetInput(image);
    recursiveMultiResolutionPyramidImageFilter->SetNumberOfLevels(numberOfLevels);
    recursiveMultiResolutionPyramidImageFilter->Update();

    // This outputs the levels (0 is the lowest resolution)
    for (unsigned int i = 0; i < numberOfLevels; ++i)
    {
      mitk::Image::Pointer outputImage = mitk::Image::New();
      CastToMitkImage(recursiveMultiResolutionPyramidImageFilter->GetOutput(i), outputImage);
      resultImages.push_back(outputImage);
    }
  }
  else {
    typename ImageTypeFilterType::Pointer recursiveMultiResolutionPyramidImageFilter = ImageTypeFilterType::New();
    recursiveMultiResolutionPyramidImageFilter->SetInput(image);
    recursiveMultiResolutionPyramidImageFilter->SetNumberOfLevels(numberOfLevels);
    recursiveMultiResolutionPyramidImageFilter->Update();

    // This outputs the levels (0 is the lowest resolution)
    for (unsigned int i = 0; i < numberOfLevels; ++i)
    {
      mitk::Image::Pointer outputImage = mitk::Image::New();
      CastToMitkImage(recursiveMultiResolutionPyramidImageFilter->GetOutput(i), outputImage);
      resultImages.push_back(outputImage);
    }
  }
}

std::vector<mitk::Image::Pointer> mitk::TransformationOperation::MultiResolution(Image::Pointer & image, unsigned int numberOfLevels, bool outputAsDouble)
{
  std::vector<Image::Pointer> resultImages;
  AccessByItk_n(image, ExecuteMultiResolution, (numberOfLevels, outputAsDouble, resultImages));
  return resultImages;
}




template<typename TPixel, unsigned int VImageDimension>
static void ExecuteLaplacianOfGaussian(itk::Image<TPixel, VImageDimension>* image, double sigma, bool outputAsDouble, mitk::Image::Pointer &resultImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> DoubleOutputType;
  typedef itk::LaplacianRecursiveGaussianImageFilter<ImageType, ImageType> ImageTypeFilterType;
  typedef itk::LaplacianRecursiveGaussianImageFilter<ImageType, DoubleOutputType> DoubleTypeFilterType;

  if (outputAsDouble)
  {
    typename DoubleTypeFilterType::Pointer filter = DoubleTypeFilterType::New();
    filter->SetInput(image);
    filter->SetSigma(sigma);
    filter->Update();

    CastToMitkImage(filter->GetOutput(), resultImage);
  }
  else {
    typename ImageTypeFilterType::Pointer filter = ImageTypeFilterType::New();
    filter->SetInput(image);
    filter->SetSigma(sigma);
    filter->Update();

    CastToMitkImage(filter->GetOutput(), resultImage);
  }
}

mitk::Image::Pointer mitk::TransformationOperation::LaplacianOfGaussian(Image::Pointer & image, double sigma, bool outputAsDouble)
{
  Image::Pointer resultImage;
  AccessByItk_n(image, ExecuteLaplacianOfGaussian, (sigma, outputAsDouble, resultImage));
  return resultImage;
}



template<typename TInputPixel, typename TOutputPixel, unsigned int VImageDimension, typename TWaveletFunction >
static void ExecuteSpecificWaveletTransformation(itk::Image<TInputPixel, VImageDimension>* image, unsigned int numberOfLevels, unsigned int numberOfBands, mitk::BorderCondition condition, std::vector<mitk::Image::Pointer> &resultImages)
{
  const unsigned int Dimension = VImageDimension;
  typedef TInputPixel                               PixelType;
  typedef TOutputPixel                              OutputPixelType;
  typedef itk::Image< PixelType, Dimension >        ImageType;
  typedef itk::Image< double, Dimension >           DoubleImageType;
  typedef itk::Image< OutputPixelType, Dimension >  OutputImageType;

  typedef itk::CastImageFilter< ImageType, DoubleImageType >                                          CastFilterType;
  typedef itk::FFTPadImageFilter< DoubleImageType >                                                   FFTPadType;
  typedef itk::ForwardFFTImageFilter< DoubleImageType, itk::Image< std::complex<double>, Dimension> > FFTFilterType;
  typedef typename FFTFilterType::OutputImageType                                                     ComplexImageType;

  typedef TWaveletFunction                                                                            WaveletFunctionType;
  typedef itk::WaveletFrequencyFilterBankGenerator< ComplexImageType, WaveletFunctionType >           WaveletFilterBankType;
  typedef itk::WaveletFrequencyForward< ComplexImageType, ComplexImageType, WaveletFilterBankType >   ForwardWaveletType;

  typedef itk::InverseFFTImageFilter< ComplexImageType, OutputImageType > InverseFFTFilterType;

  // Convert input parameter
  unsigned int highSubBands = numberOfBands; //inputBands;
  unsigned int levels = numberOfLevels;


  // Perform FFT on input image
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(image);

  // Pad Image so it fits the expect
  typename FFTPadType::Pointer fftpad = FFTPadType::New();
  // Pad to powers of two: every level halves the size and the VNL FFT only
  // accepts sizes with the prime factors 2, 3 and 5, so any other padding
  // can end up at a prime size after a few levels.
  fftpad->SetSizeGreatestPrimeFactor(2);
  itk::ConstantBoundaryCondition< DoubleImageType > constantBoundaryCondition;
  itk::PeriodicBoundaryCondition< DoubleImageType > periodicBoundaryCondition;
  itk::ZeroFluxNeumannBoundaryCondition< DoubleImageType > zeroFluxNeumannBoundaryCondition;
  switch (condition)
  {
  case mitk::BorderCondition::Constant:
    fftpad->SetBoundaryCondition(&constantBoundaryCondition);
    break;
  case mitk::BorderCondition::Periodic:
    fftpad->SetBoundaryCondition(&periodicBoundaryCondition);
    break;
  case mitk::BorderCondition::ZeroFluxNeumann:
    fftpad->SetBoundaryCondition(&zeroFluxNeumannBoundaryCondition);
    break;
  default:
    break;
  }
  fftpad->SetInput(castFilter->GetOutput());

  // FFTPadImageFilter shifts the start index of the padded region to negative
  // values. WaveletFrequencyForward scales requested regions per level from
  // that index and ends up outside the largest possible region, so the padded
  // image is moved back to index zero first. The padding is centered, so the
  // origin has to follow the same shift or the content would be displaced by
  // half the padding.
  fftpad->UpdateOutputInformation();
  const auto paddedIndex = fftpad->GetOutput()->GetLargestPossibleRegion().GetIndex();
  typename DoubleImageType::PointType paddedOrigin;
  fftpad->GetOutput()->TransformIndexToPhysicalPoint(paddedIndex, paddedOrigin);

  typedef itk::ChangeInformationImageFilter< DoubleImageType > ChangeInformationType;
  typename ChangeInformationType::Pointer changeInformation = ChangeInformationType::New();
  changeInformation->SetInput(fftpad->GetOutput());
  changeInformation->ChangeRegionOn();
  typename ChangeInformationType::OutputImageOffsetType regionOffset;
  for (unsigned int i = 0; i < Dimension; ++i)
  {
    regionOffset[i] = -paddedIndex[i];
  }
  changeInformation->SetOutputOffset(regionOffset);
  changeInformation->ChangeOriginOn();
  changeInformation->SetOutputOrigin(paddedOrigin);

  typename FFTFilterType::Pointer fftFilter = FFTFilterType::New();
  fftFilter->SetInput(changeInformation->GetOutput());

  // Calculate forward transformation
  typename ForwardWaveletType::Pointer forwardWavelet = ForwardWaveletType::New();

  forwardWavelet->SetHighPassSubBands(highSubBands);
  forwardWavelet->SetLevels(levels);
  forwardWavelet->SetInput(fftFilter->GetOutput());
  forwardWavelet->Update();

  // Obtain target spacing and origin. Both refer to the padded image, which is
  // the grid the sub-bands are derived from.
  changeInformation->UpdateOutputInformation();
  const DoubleImageType* paddedImage = changeInformation->GetOutput();
  const auto paddedSpacing = paddedImage->GetSpacing();

  typename OutputImageType::SpacingType expectedSpacing;
  typename OutputImageType::PointType expectedOrigin;

  // Inverse FFT to obtain filtered images
  for (unsigned int level = 0; level < numberOfLevels + 1; ++level)
  {
    double scaleFactorPerLevel = std::pow(static_cast< double >(forwardWavelet->GetScaleFactor()),static_cast< double >(level));

    // Each level merges scaleFactorPerLevel voxels into one, so the center of
    // the new corner voxel lies at that continuous index of the padded grid.
    itk::ContinuousIndex< double, Dimension > cornerIndex;
    for (unsigned int i = 0; i < Dimension; ++i)
    {
      expectedSpacing[i] = paddedSpacing[i] * scaleFactorPerLevel;
      cornerIndex[i] = -0.5 + 0.5 * scaleFactorPerLevel;
    }
    paddedImage->TransformContinuousIndexToPhysicalPoint(cornerIndex, expectedOrigin);
    for (unsigned int band = 0; band < highSubBands; ++band)
    {
      unsigned int nOutput = level * forwardWavelet->GetHighPassSubBands() + band;
      // Do not compute bands in low-pass level.
      if (level == numberOfLevels && band == 0)
      {
        nOutput = forwardWavelet->GetTotalOutputs() - 1;
      }
      else if (level == numberOfLevels && band != 0)
      {
        break;
      }

      // The sub-band images shrink with each level, so a fresh filter is needed
      // per output; reusing one keeps the requested region of the previous size.
      typename InverseFFTFilterType::Pointer inverseFFT = InverseFFTFilterType::New();
      inverseFFT->SetInput(forwardWavelet->GetOutput(nOutput));
      inverseFFT->Update();

      auto itkOutputImage = inverseFFT->GetOutput();
      itkOutputImage->SetSpacing(expectedSpacing);
      itkOutputImage->SetOrigin(expectedOrigin);
      itkOutputImage->SetDirection(paddedImage->GetDirection());
      mitk::Image::Pointer outputImage = mitk::Image::New();
      CastToMitkImage(itkOutputImage, outputImage);
      resultImages.push_back(outputImage);
    }
  }
}

template<typename TPixel, unsigned int VImageDimension>
static void ExecuteWaveletTransformation(itk::Image<TPixel, VImageDimension>* image, unsigned int numberOfLevels, unsigned int numberOfBands, mitk::BorderCondition condition, mitk::WaveletType waveletType, std::vector<mitk::Image::Pointer> &resultImages)
{
  typedef itk::Point< double, VImageDimension >                                   PointType;
  typedef itk::HeldIsotropicWavelet< double, VImageDimension, PointType >       HeldIsotropicWaveletType;
  typedef itk::VowIsotropicWavelet< double, VImageDimension, PointType >        VowIsotropicWaveletType;
  typedef itk::SimoncelliIsotropicWavelet< double, VImageDimension, PointType > SimoncelliIsotropicWaveletType;
  typedef itk::ShannonIsotropicWavelet< double, VImageDimension, PointType >    ShannonIsotropicWaveletType;

  switch (waveletType)
  {
  case mitk::WaveletType::Held:
    ExecuteSpecificWaveletTransformation<TPixel, double, VImageDimension, HeldIsotropicWaveletType >(image, numberOfLevels, numberOfBands, condition, resultImages);
    break;
  case mitk::WaveletType::Shannon:
    ExecuteSpecificWaveletTransformation<TPixel, double, VImageDimension, ShannonIsotropicWaveletType >(image, numberOfLevels, numberOfBands, condition, resultImages);
    break;
  case mitk::WaveletType::Simoncelli:
    ExecuteSpecificWaveletTransformation<TPixel, double, VImageDimension, SimoncelliIsotropicWaveletType >(image, numberOfLevels, numberOfBands, condition, resultImages);
    break;
  case mitk::WaveletType::Vow:
    ExecuteSpecificWaveletTransformation<TPixel, double, VImageDimension, VowIsotropicWaveletType >(image, numberOfLevels, numberOfBands, condition, resultImages);
    break;
  default:
    ExecuteSpecificWaveletTransformation<TPixel, double, VImageDimension, ShannonIsotropicWaveletType >(image, numberOfLevels, numberOfBands, condition, resultImages);
    break;
  }
}

std::vector<mitk::Image::Pointer> mitk::TransformationOperation::WaveletForward(Image::Pointer & image, unsigned int numberOfLevels, unsigned int numberOfBands, mitk::BorderCondition condition, mitk::WaveletType waveletType)
{
  // MITK does not use ITK's automatic factory registration, so the FFT
  // backend has to be registered explicitly before any FFT filter is created.
  // ITK appends to its factory list without checking for duplicates, so this
  // must happen exactly once per process.
  static std::once_flag fftFactoryRegistered;
  std::call_once(fftFactoryRegistered, []() { itk::VnlFFTImageFilterInitFactory::RegisterFactories(); });

  std::vector<Image::Pointer> resultImages;
  AccessByItk_n(image, ExecuteWaveletTransformation, (numberOfLevels, numberOfBands, condition, waveletType, resultImages));
  return resultImages;
}


template<typename TPixel, unsigned int VImageDimension>
static void ExecuteImageTypeToDouble(itk::Image<TPixel, VImageDimension>* image, mitk::Image::Pointer &outputImage)
{
  typedef itk::Image< TPixel, VImageDimension >        ImageType;
  typedef itk::Image< double, VImageDimension >        DoubleImageType;
  typedef itk::CastImageFilter< ImageType, DoubleImageType >    CastFilterType;
  typedef itk::ImageDuplicator< DoubleImageType >    DuplicatorType;

  // Perform FFT on input image
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(image);
  castFilter->Update();
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(castFilter->GetOutput());
  duplicator->Update();
  CastToMitkImage(duplicator->GetOutput(), outputImage);
}

template<typename TPixel, unsigned int VImageDimension>
static void ExecuteRoundImage(itk::Image<TPixel, VImageDimension>* /*image*/, mitk::Image::Pointer resampledImage,  mitk::Image::Pointer &outputImage)
{
  typedef itk::Image< TPixel, VImageDimension >        ImageType;
  typedef itk::Image< double, VImageDimension >        DoubleImageType;
  typedef itk::UnaryFunctorImageFilter< DoubleImageType, ImageType, mitk::Functor::RoundValue<double, TPixel> > DefaultFilterType;

  typename DoubleImageType::Pointer itkImage = DoubleImageType::New();
  mitk::CastToItkImage(resampledImage, itkImage);

  typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
  filter->SetInput(itkImage);
  filter->Update();

  CastToMitkImage(filter->GetOutput(), outputImage);
}

mitk::Image::Pointer mitk::TransformationOperation::ResampleImage(Image::Pointer &image, mitk::Vector3D spacingVector, mitk::ImageMappingInterpolator::Type interpolator, GridInterpolationPositionType position, bool returnAsDouble, bool roundOutput)
{
  // Convert image to double if required
  mitk::Image::Pointer tmpImage = image;
  if (returnAsDouble)
  {
    AccessByItk_n(image, ExecuteImageTypeToDouble, (tmpImage));
  }

  auto newGeometry = image->GetGeometry()->Clone();
  mitk::Vector3D spacing;
  mitk::BaseGeometry::BoundsArrayType bounds = newGeometry->GetBounds();
  for (int i = 0; i < 3; ++i)
  {
    spacing[i] = newGeometry->GetSpacing()[i];
    //bounds[i*2+1] = newGeometry->GetBounds()[i * 2 + 1];
    if (spacingVector[i] > 0)
    {
      spacing[i] = spacingVector[i];
      if (position == mitk::GridInterpolationPositionType::SameSize)
      {
        unsigned int samples = image->GetDimensions()[i];
        double currentSpacing = newGeometry->GetSpacing()[i];
        double newFactor = std::floor(samples*currentSpacing / spacingVector[i]);
        spacing[i] = samples * currentSpacing / newFactor;
      }
    }
    bounds[i * 2] = 0;
    bounds[i*2+1] = std::ceil(bounds[i*2+1] * newGeometry->GetSpacing()[i] *1.0 / spacing[i]);
  }
  mitk::Point3D origin = newGeometry->GetOrigin();
  if (position == mitk::GridInterpolationPositionType::CenterAligned)
  {
    for (int i = 0; i < 3; ++i)
    {
      double oldLength = newGeometry->GetSpacing()[i] * newGeometry->GetBounds()[i*2+1];
      double newLength = spacing[i] * bounds[i*2+1];
      origin[i] = origin[i] - (newLength - oldLength) / 2;
    }
  }

  newGeometry->SetSpacing(spacing);
  newGeometry->SetOrigin(origin);
  newGeometry->SetBounds(bounds);

  mitk::Image::Pointer tmpResult = ImageMappingHelper::map(
    tmpImage,
    mitk::GenerateIdentityRegistration3D().GetPointer(),
    false,
    0.0, //Padding Value
    newGeometry.GetPointer(),
    false,
    0, //Error Value
    interpolator
  );

  mitk::Image::Pointer result = mitk::Image::New();

  if (roundOutput)
  {
    AccessByItk_n(tmpImage, ExecuteRoundImage, (tmpResult, result));
  }
  else
  {
    result = tmpResult;
  }

  return result;
}


template<typename TPixel, unsigned int VImageDimension>
static void ExecuteImageThresholding(itk::Image<TPixel, VImageDimension>* image, mitk::Image::Pointer &resultImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::UnaryFunctorImageFilter< ImageType, MaskType, mitk::Functor::ThresholdValue<TPixel> > DefaultFilterType;

  typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
  filter->SetInput(image);
  filter->GetFunctor().value = 0.5;
  filter->Update();

  CastToMitkImage(filter->GetOutput(), resultImage);

}

mitk::Image::Pointer mitk::TransformationOperation::ResampleMask(Image::Pointer &image, mitk::Vector3D spacingVector, mitk::ImageMappingInterpolator::Type interpolator, GridInterpolationPositionType position)
{
  mitk::Image::Pointer result;
  if (interpolator == mitk::ImageMappingInterpolator::NearestNeighbor)
  {
    result = TransformationOperation::ResampleImage(image, spacingVector, interpolator, position, false, false);
  }
  else
  {
    auto tmpResult = TransformationOperation::ResampleImage(image, spacingVector, interpolator, position, true, false);
    AccessByItk_n(tmpResult, ExecuteImageThresholding, (result));
  }
  return result;
}

namespace itk
{
  namespace utils
  {

    IndexPairType IndexToLevelBandSteerablePyramid(unsigned int linearIndex,
      unsigned int levels, unsigned int bands)
    {
      unsigned int totalOutputs = 1 + levels * bands;
      if (linearIndex > totalOutputs - 1)
      {
        itkGenericExceptionMacro(<< "Failed converting linearIndex " << linearIndex
          << " with levels: " << levels << " bands: " << bands <<
          " to Level,Band pair : out of bounds");
      }

      // Low pass (band = 0).
      if (linearIndex == totalOutputs - 1)
      {
        return std::make_pair(levels - 1, 0);
      }

      unsigned int band = (linearIndex) % bands + 1;
      // note integer division ahead.
      unsigned int level = (linearIndex) / bands;
      itkAssertInDebugAndIgnoreInReleaseMacro(level < levels);
      return std::make_pair(level, band);
    }

    // Instantiation
    template
      unsigned int ComputeMaxNumberOfLevels<3>(const Size< 3 >& inputSize, const unsigned int & scaleFactor);

    template
      unsigned int ComputeMaxNumberOfLevels<2>(const Size< 2 >& inputSize, const unsigned int & scaleFactor);
  } // end namespace utils
} // end namespace itk
