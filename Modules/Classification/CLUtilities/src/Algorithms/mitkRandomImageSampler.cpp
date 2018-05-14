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

#include "mitkRandomImageSampler.h"
#include <itkBilateralImageFilter.h>
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "itkUnaryFunctorImageFilter.h"
#include <itkImageRandomNonRepeatingIteratorWithIndex.h>
#include "itkImageDuplicator.h"

mitk::RandomImageSampler::RandomImageSampler()
  : m_AcceptanceRate(0.1), m_SamplingMode(RandomImageSamplerMode::SINGLE_ACCEPTANCE_RATE)
{
  //default parameters DomainSigma: 2 , RangeSigma: 50, AutoKernel: true, KernelRadius: 1
}

mitk::RandomImageSampler::~RandomImageSampler()
{
}

template< class TInput, class TOutput>
class RandomlySampleFunctor
{
public:
  RandomlySampleFunctor() {};
  ~RandomlySampleFunctor() {};
  bool operator!=(const RandomlySampleFunctor &) const
  {
    return false;
  }
  bool operator==(const RandomlySampleFunctor & other) const
  {
    return !(*this != other);
  }
  inline TOutput operator()(const TInput & A) const
  {
    if (rand() < RAND_MAX*m_AcceptanceRate)
      return A;
    else
      return TOutput(0);
  }

  double m_AcceptanceRate = 0.1;
};

template< class TInput, class TOutput>
class RandomlySampleClassDependedFunctor
{
public:
  RandomlySampleClassDependedFunctor() {};
  ~RandomlySampleClassDependedFunctor() {};
  bool operator!=(const RandomlySampleClassDependedFunctor &) const
  {
    return false;
  }
  bool operator==(const RandomlySampleClassDependedFunctor & other) const
  {
    return !(*this != other);
  }
  inline TOutput operator()(const TInput & A) const
  {
    std::size_t index = static_cast<std::size_t>(A + 0.5);
    double samplingRate = 0;
    if (index >= 0 && index < m_SamplingRateVector.size())
    {
      samplingRate = m_SamplingRateVector[index];
    }

    if (rand() < RAND_MAX*samplingRate)
      return A;
    else
      return TOutput(0);
  }

  std::vector<double> m_SamplingRateVector;
};

void mitk::RandomImageSampler::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetInput(0);
  switch (m_SamplingMode)
  {
  case SINGLE_ACCEPTANCE_RATE:
    AccessByItk(inputImage.GetPointer(), ItkImageProcessing);
    break;
  case CLASS_DEPENDEND_ACCEPTANCE_RATE :
    AccessByItk(inputImage.GetPointer(), ItkImageProcessingClassDependendSampling);
    break;
  case SINGLE_NUMBER_OF_ACCEPTANCE:
    AccessByItk(inputImage.GetPointer(), ItkImageProcessingFixedNumberSampling);
    break;
  case CLASS_DEPENDEND_NUMBER_OF_ACCEPTANCE:
    AccessByItk(inputImage.GetPointer(), ItkImageProcessingClassDependendNumberSampling);
    break;
  default:
    AccessByItk(inputImage.GetPointer(), ItkImageProcessing);
    break;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::RandomImageSampler::ItkImageProcessing( const itk::Image<TPixel,VImageDimension>* itkImage )
{
  //ITK Image type given from the input image
  typedef itk::Image< TPixel, VImageDimension >   ItkImageType;
  //bilateral filter with same type
  typedef RandomlySampleFunctor< typename ItkImageType::PixelType,
    typename ItkImageType::PixelType> LocalSampleFunctorType;
  typedef itk::UnaryFunctorImageFilter<ItkImageType, ItkImageType, LocalSampleFunctorType > RandomImageSamplerType;
  typename RandomImageSamplerType::Pointer RandomImageSampler = RandomImageSamplerType::New();
  RandomImageSampler->SetInput(itkImage);

  LocalSampleFunctorType functor;
  functor.m_AcceptanceRate = m_AcceptanceRate;
  RandomImageSampler->SetFunctor(functor);
  RandomImageSampler->GetFunctor().m_AcceptanceRate = m_AcceptanceRate;
  RandomImageSampler->Update();


  //get  Pointer to output image
  mitk::Image::Pointer resultImage = this->GetOutput();
  //write into output image
  mitk::CastToMitkImage(RandomImageSampler->GetOutput(), resultImage);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::RandomImageSampler::ItkImageProcessingClassDependendSampling(const itk::Image<TPixel, VImageDimension>* itkImage)
{
  //ITK Image type given from the input image
  typedef itk::Image< TPixel, VImageDimension > ItkImageType;
  //bilateral filter with same type
  typedef RandomlySampleClassDependedFunctor< typename ItkImageType::PixelType,
    typename ItkImageType::PixelType> LocalSampleFunctorType;
  typedef itk::UnaryFunctorImageFilter<ItkImageType, ItkImageType, LocalSampleFunctorType > RandomImageSamplerType;
  typename RandomImageSamplerType::Pointer RandomImageSampler = RandomImageSamplerType::New();
  RandomImageSampler->SetInput(itkImage);

  LocalSampleFunctorType functor;
  functor.m_SamplingRateVector = m_AcceptanceRateVector;
  RandomImageSampler->SetFunctor(functor);
  RandomImageSampler->GetFunctor().m_SamplingRateVector = m_AcceptanceRateVector;
  RandomImageSampler->Update();


  //get  Pointer to output image
  mitk::Image::Pointer resultImage = this->GetOutput();
  //write into output image
  mitk::CastToMitkImage(RandomImageSampler->GetOutput(), resultImage);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::RandomImageSampler::ItkImageProcessingFixedNumberSampling(const itk::Image<TPixel, VImageDimension>* itkImage)
{
  //ITK Image type given from the input image
  typedef itk::Image< TPixel, VImageDimension >   ItkImageType;
  typedef itk::ImageDuplicator< ItkImageType >       DuplicatorType;
  typedef itk::ImageRandomNonRepeatingIteratorWithIndex <ItkImageType> IteratorType;

  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(itkImage);
  duplicator->Update();
  typename ItkImageType::Pointer clonedImage = duplicator->GetOutput();

  //clonedImage->FillBuffer(0);
  std::vector<unsigned int> counts;
  IteratorType iter(clonedImage, clonedImage->GetLargestPossibleRegion());
  iter.SetNumberOfSamples(clonedImage->GetLargestPossibleRegion().GetNumberOfPixels());
  //iter.GoToBegin();
  while (!iter.IsAtEnd())
  {
    std::size_t index = static_cast<std::size_t>(iter.Value() + 0.5);
    while (index >= counts.size())
    {
      counts.push_back(0);
    }
    if (counts[index] < m_NumberOfSamples)
    {
      //clonedImage->SetPixel(iter.GetIndex(), iter.Value());
      counts[index] += 1;
    }
    else
    {
      iter.Set(0.0);
      //clonedImage->SetPixel(iter.GetIndex(), 0.0);
    }

    ++iter;
  }

  //get  Pointer to output image
  mitk::Image::Pointer resultImage = this->GetOutput();
  //write into output image
  mitk::CastToMitkImage(clonedImage, resultImage);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::RandomImageSampler::ItkImageProcessingClassDependendNumberSampling(const itk::Image<TPixel, VImageDimension>* itkImage)
{
  //ITK Image type given from the input image
  typedef itk::Image< TPixel, VImageDimension >   ItkImageType;
  typedef itk::ImageDuplicator< ItkImageType >       DuplicatorType;
  typedef itk::ImageRandomNonRepeatingIteratorWithIndex<ItkImageType> IteratorType;

  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(itkImage);
  duplicator->Update();
  typename ItkImageType::Pointer clonedImage = duplicator->GetOutput();

  std::vector<unsigned int> counts;
  IteratorType iter(clonedImage, clonedImage->GetLargestPossibleRegion());
  iter.SetNumberOfSamples(clonedImage->GetLargestPossibleRegion().GetNumberOfPixels());
  while (!iter.IsAtEnd())
  {
    std::size_t index = static_cast<std::size_t>(iter.Value() + 0.5);
    if (index < m_NumberOfSamplesVector.size())
    {
      while (index >= counts.size())
      {
        counts.push_back(0);
      }

      if (counts[index] < m_NumberOfSamplesVector[index])
      {
        counts[index] += 1;
      }
      else
      {
        iter.Set(0.0);
      }
    }
    else
    {
      iter.Set(0.0);
    }

    ++iter;
  }

  //get  Pointer to output image
  mitk::Image::Pointer resultImage = this->GetOutput();
  //write into output image
  mitk::CastToMitkImage(clonedImage, resultImage);
}


void mitk::RandomImageSampler::GenerateOutputInformation()
{
  mitk::Image::Pointer inputImage  = (mitk::Image*) this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();
  itkDebugMacro(<<"GenerateOutputInformation()");
  if(inputImage.IsNull()) return;
}
