/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkStitchImageFilter_hxx
#define itkStitchImageFilter_hxx

#include "itkStitchImageFilter.h"
#include "itkObjectFactory.h"
#include "itkIdentityTransform.h"
#include "itkProgressReporter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageScanlineIterator.h"
#include "itkSpecialCoordinatesImage.h"
#include "itkDefaultConvertPixelTraits.h"
#include "itkSimpleDataObjectDecorator.h"

#include <numeric>

namespace itk
{

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::StitchImageFilter() :
  m_OutputSpacing( 1.0 ),
  m_OutputOrigin( 0.0 ),
  m_UseReferenceImage( false )
{

  m_Size.Fill( 0 );
  m_OutputStartIndex.Fill( 0 );

  m_OutputDirection.SetIdentity();

  // Pipeline input configuration

  // implicit input index set:
  // #1 "ReferenceImage" optional
  Self::AddOptionalInputName("ReferenceImage");

  m_DefaultPixelValue
    = NumericTraits<PixelType>::ZeroValue( m_DefaultPixelValue );
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::SetInput(unsigned int index, const InputImageType* image)
{
  this->SetInput(index, image, itk::IdentityTransform< TTransformPrecisionType, ImageDimension>::New().GetPointer(), LinearInterpolatorType::New().GetPointer());
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::SetInput(unsigned int index, const InputImageType* image, const TransformType* transform)
{
  this->SetInput(index, image, transform, LinearInterpolatorType::New().GetPointer());
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::SetInput(unsigned int index, const InputImageType* image, const TransformType* transform, InterpolatorType* interpolator)
{
  Superclass::SetInput(index, image);
  m_Interpolators[image] = interpolator;

  this->SetTransform(index, transform);
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
  void
  StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
  ::SetTransform(unsigned int index, const TransformType* transform)
{
  const auto transformName = this->GetTransformInputName(index);
  typedef SimpleDataObjectDecorator< TransformPointerType > DecoratorType;
  const DecoratorType* oldInput = itkDynamicCastInDebugMode< const DecoratorType* >(this->ProcessObject::GetInput(transformName));
  if (!oldInput || oldInput->Get() != transform)
  {
    typename DecoratorType::Pointer newInput = DecoratorType::New();
    // Process object is not const-correct so the const_cast is required here
    newInput->Set(const_cast<TransformType*>(transform));
    this->ProcessObject::SetInput(transformName, newInput);
  }
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
  const typename StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::TransformType*
  StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
  ::GetTransform(unsigned int index) const
{
  typedef SimpleDataObjectDecorator< TransformPointerType > DecoratorType;
  const DecoratorType* input = itkDynamicCastInDebugMode< const DecoratorType* >(this->ProcessObject::GetInput(this->GetTransformInputName(index)));

  if (nullptr != input)
  {
    return input->Get();
  }

  return nullptr;
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
  const typename StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >::InterpolatorType*
  StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
  ::GetInterpolator(unsigned int index) const
{
  auto input = this->GetInput(index);
  if (m_Interpolators.find(input) != std::end(m_Interpolators))
  {
    return m_Interpolators[input];
  }

  return nullptr;
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::SetOutputSpacing(const double *spacing)
{
  SpacingType s;
  for(unsigned int i = 0; i < TOutputImage::ImageDimension; ++i)
    {
    s[i] = static_cast< typename SpacingType::ValueType >(spacing[i]);
    }
  this->SetOutputSpacing(s);
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::SetOutputOrigin(const double *origin)
{
  OriginPointType p(origin);

  this->SetOutputOrigin(p);
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::SetOutputParametersFromImage(const ImageBaseType *image)
{
  this->SetOutputOrigin ( image->GetOrigin() );
  this->SetOutputSpacing ( image->GetSpacing() );
  this->SetOutputDirection ( image->GetDirection() );
  this->SetOutputStartIndex ( image->GetLargestPossibleRegion().GetIndex() );
  this->SetSize ( image->GetLargestPossibleRegion().GetSize() );
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::BeforeThreadedGenerateData()
{
  this->EnsureInterpolators();
  this->EnsureTransforms();

  unsigned int nComponents
    = DefaultConvertPixelTraits<PixelType>::GetNumberOfComponents(
        m_DefaultPixelValue );

  if (nComponents == 0)
    {
    PixelComponentType zeroComponent
      = NumericTraits<PixelComponentType>::ZeroValue( zeroComponent );
    nComponents = this->GetInput()->GetNumberOfComponentsPerPixel();
    NumericTraits<PixelType>::SetLength(m_DefaultPixelValue, nComponents );
    for (unsigned int n=0; n<nComponents; n++)
      {
      PixelConvertType::SetNthComponent( n, m_DefaultPixelValue,
                                         zeroComponent );
      }
    }
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::AfterThreadedGenerateData()
{
  // Disconnect input image from the interpolator
  for (auto& interpolator : m_Interpolators)
  {
    interpolator.second->SetInputImage(ITK_NULLPTR);
  }
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       ThreadIdType threadId)
{
  // Check whether the input or the output is a
  // SpecialCoordinatesImage. If either are, then we cannot use the
  // fast path since index mapping will definitely not be linear.
  typedef SpecialCoordinatesImage< PixelType, ImageDimension >           OutputSpecialCoordinatesImageType;
  typedef SpecialCoordinatesImage< InputPixelType, InputImageDimension > InputSpecialCoordinatesImageType;

  if( outputRegionForThread.GetNumberOfPixels() == 0 )
    {
    return;
    }

  this->NonlinearThreadedGenerateData(outputRegionForThread, threadId);
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
typename StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::PixelType
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::CastPixelWithBoundsChecking(const InterpolatorOutputType value,
                              const ComponentType minComponent,
                              const ComponentType maxComponent ) const
{
  const unsigned int nComponents = InterpolatorConvertType::GetNumberOfComponents(value);
  PixelType          outputValue;

  NumericTraits<PixelType>::SetLength( outputValue, nComponents );

  for (unsigned int n = 0; n < nComponents; n++)
    {
    ComponentType component = InterpolatorConvertType::GetNthComponent( n, value );

    if ( component < minComponent )
      {
      PixelConvertType::SetNthComponent( n, outputValue, static_cast<PixelComponentType>( minComponent ) );
      }
    else if ( component > maxComponent )
      {
      PixelConvertType::SetNthComponent( n, outputValue, static_cast<PixelComponentType>( maxComponent ) );
      }
    else
      {
      PixelConvertType::SetNthComponent(n, outputValue,
                                        static_cast<PixelComponentType>( component ) );
      }
    }

  return outputValue;
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::NonlinearThreadedGenerateData(const OutputImageRegionType &
                                outputRegionForThread,
                                ThreadIdType threadId)
{
  // Get the output pointers
  OutputImageType *outputPtr = this->GetOutput();

  // Get this input pointers
  InputImageVectorType inputs = this->GetInputs();
  TransformMapType transforms = this->GetTransforms();

  // Create an iterator that will walk the output region for this thread.
  typedef ImageRegionIteratorWithIndex< TOutputImage > OutputIterator;
  OutputIterator outIt(outputPtr, outputRegionForThread);

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  PointType outputPoint;         // Coordinates of current output pixel
  PointType inputPoint;          // Coordinates of current input pixel

  ContinuousInputIndexType inputIndex;

  // Support for progress methods/callbacks
  ProgressReporter progress( this,
                             threadId,
                             outputRegionForThread.GetNumberOfPixels() );

  // Min/max values of the output pixel type AND these values
  // represented as the output type of the interpolator
  const PixelComponentType minValue =  NumericTraits< PixelComponentType >::NonpositiveMin();
  const PixelComponentType maxValue =  NumericTraits< PixelComponentType >::max();

  typedef typename InterpolatorType::OutputType OutputType;
  const ComponentType minOutputValue = static_cast< ComponentType >( minValue );
  const ComponentType maxOutputValue = static_cast< ComponentType >( maxValue );

  // Walk the output region
  outIt.GoToBegin();

  while ( !outIt.IsAtEnd() )
  {
    // Determine the index of the current output pixel
    outputPtr->TransformIndexToPhysicalPoint(outIt.GetIndex(), outputPoint);

    std::vector<PixelType> pixvals;

    for (const auto& input : inputs)
    {
      // Compute corresponding input pixel position
      inputPoint = transforms[input]->TransformPoint(outputPoint);
      const bool isInsideInput = input->TransformPhysicalPointToContinuousIndex(inputPoint, inputIndex);

      // Evaluate input at right position and copy to the output
      if (m_Interpolators[input]->IsInsideBuffer(inputIndex) && isInsideInput)
      {
        OutputType value = m_Interpolators[input]->EvaluateAtContinuousIndex(inputIndex);
        pixvals.emplace_back(this->CastPixelWithBoundsChecking(value, minOutputValue, maxOutputValue));
      }
    }

    if (!pixvals.empty())
    { //at least one input provided a value -> compute weighted sum
      double sum = std::accumulate(pixvals.begin(), pixvals.end(), 0.0);
      outIt.Set(sum / pixvals.size());
    }
    else
    {
      outIt.Set(m_DefaultPixelValue); // default background value
    }

    progress.CompletedPixel();
    ++outIt;
  }
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::LinearThreadedGenerateData(const OutputImageRegionType &
                             outputRegionForThread,
                             ThreadIdType threadId)
{
  // Get the output pointers
  OutputImageType *outputPtr = this->GetOutput();

  // Get this input pointers
  InputImageVectorType inputs = this->GetInputs();
  TransformMapType transforms = this->GetTransforms();

  // Create an iterator that will walk the output region for this thread.
  typedef ImageScanlineIterator< TOutputImage > OutputIterator;

  OutputIterator outIt(outputPtr, outputRegionForThread);

  // Define a few indices that will be used to translate from an input pixel
  // to an output pixel
  PointType outputPoint;         // Coordinates of current output pixel
  PointType tmpOutputPoint;

  std::map<const InputImageType*, ContinuousInputIndexType> inputIndices;

  typedef typename PointType::VectorType VectorType;
  std::map<const InputImageType*, VectorType> deltas; // delta in input continuous index coordinate frame

  IndexType index;

  const typename OutputImageRegionType::SizeType &regionSize = outputRegionForThread.GetSize();
  const SizeValueType numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / regionSize[0];

  // Support for progress methods/callbacks
  ProgressReporter progress( this,
                             threadId,
                             numberOfLinesToProcess );

  typedef typename InterpolatorType::OutputType OutputType;

  // Cache information from the superclass
  PixelType defaultValue = this->GetDefaultPixelValue();

  // Min/max values of the output pixel type AND these values
  // represented as the output type of the interpolator
  const PixelComponentType minValue =  NumericTraits< PixelComponentType >::NonpositiveMin();
  const PixelComponentType maxValue =  NumericTraits< PixelComponentType >::max();

  typedef typename InterpolatorType::OutputType OutputType;
  const ComponentType minOutputValue = static_cast< ComponentType >( minValue );
  const ComponentType maxOutputValue = static_cast< ComponentType >( maxValue );

  // Determine the position of the first pixel in the scanline
  index = outIt.GetIndex();
  outputPtr->TransformIndexToPhysicalPoint(index, outputPoint);

  // Compute corresponding input pixel position
  for (const auto& input : inputs)
  {
    auto inputPoint = transforms[input]->TransformPoint(outputPoint);

    ContinuousInputIndexType inputIndex;
    input->TransformPhysicalPointToContinuousIndex(inputPoint, inputIndex);
    inputIndices[input] = inputIndex;
  }

  // As we walk across a scan line in the output image, we trace
  // an oriented/scaled/translated line in the input image.  Cache
  // the delta along this line in continuous index space of the input
  // image. This allows us to use vector addition to model the
  // transformation.
  //
  // To find delta, we take two pixels adjacent in a scanline
  // and determine the continuous indices of these pixels when
  // mapped to the input coordinate frame. We use the difference
  // between these two continuous indices as the delta to apply
  // to an index to trace line in the input image as we move
  // across the scanline of the output image.
  //
  // We determine delta in this manner so that Images
  // are both handled properly (taking into account the direction cosines).
  //
  ++index[0];
  outputPtr->TransformIndexToPhysicalPoint(index, tmpOutputPoint);
  for (const auto& input : inputs)
  {
    auto tmpInputPoint = transforms[input]->TransformPoint(tmpOutputPoint);

    ContinuousInputIndexType tmpInputIndex;
    input->TransformPhysicalPointToContinuousIndex(tmpInputPoint,
      tmpInputIndex);
    deltas[input] = tmpInputIndex - inputIndices[input];
  }

  while ( !outIt.IsAtEnd() )
    {
    // Determine the continuous index of the first pixel of output
    // scanline when mapped to the input coordinate frame.
    //

    // First get the position of the pixel in the output coordinate frame
    index = outIt.GetIndex();
    outputPtr->TransformIndexToPhysicalPoint(index, outputPoint);

    // Compute corresponding input pixel continuous index, this index
    // will incremented in the scanline loop
    for (const auto& input : inputs)
    {
      auto inputPoint = transforms[input]->TransformPoint(outputPoint);
      input->TransformPhysicalPointToContinuousIndex(inputPoint, inputIndices[input]);
    }

    while ( !outIt.IsAtEndOfLine() )
      {
      std::vector<PixelType> pixvals;

      for (const auto& input : inputs)
      {
        if (m_Interpolators[input]->IsInsideBuffer(inputIndices[input]))
        {
          auto value = m_Interpolators[input]->EvaluateAtContinuousIndex(inputIndices[input]);
          pixvals.emplace_back(this->CastPixelWithBoundsChecking(value, minOutputValue, maxOutputValue));
        }
        inputIndices[input] += deltas[input];
      }

      // Evaluate input at right position and copy to the output
      if ( !pixvals.empty())
        {
        double sum = std::accumulate(pixvals.begin(), pixvals.end(), 0.0);
        outIt.Set(sum / pixvals.size());
        }
      else
        {
        outIt.Set(defaultValue); // default background value
        }

      ++outIt;
      }
    progress.CompletedPixel();
    outIt.NextLine();
    }
}

template<typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType, typename TTransformPrecisionType>
typename StitchImageFilter<TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType>::InputImageVectorType
StitchImageFilter<TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType>
::GetInputs()
{
  InputImageVectorType inputs;
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
  {
    auto input = this->GetInput(i);
    if (nullptr != input)
    {
      inputs.push_back(input);
    }
  }
  return inputs;
}

template<typename TInputImage, typename TOutputImage, typename TInterpolatorPrecisionType, typename TTransformPrecisionType>
typename StitchImageFilter<TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType>::TransformMapType
StitchImageFilter<TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType>
::GetTransforms()
{
  TransformMapType transforms;
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
  {
    auto input = this->GetInput(i);
    auto transform = this->GetTransform(i);
    transforms[input] = transform;
  }
  return transforms;
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::GenerateInputRequestedRegion()
{
  // Call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  if ( !this->GetInput() )
    {
    return;
    }

  // Get pointers to the input
  auto inputs = this->GetInputs();

  for (auto& input : inputs)
  {
    InputImagePointer inputPtr =
      const_cast<TInputImage*>(input);
    // Determining the actual input region is non-trivial, especially
    // when we cannot assume anything about the transform being used.
    // So we do the easy thing and request the entire input image.
    //
    inputPtr->SetRequestedRegionToLargestPossibleRegion();
  }
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::GenerateOutputInformation()
{
  // Call the superclass' implementation of this method
  Superclass::GenerateOutputInformation();

  // Get pointers to the input and output
  OutputImageType *outputPtr = this->GetOutput();
  if ( !outputPtr )
    {
    return;
    }

  const ReferenceImageBaseType *referenceImage = this->GetReferenceImage();

  // Set the size of the output region
  if ( m_UseReferenceImage && referenceImage )
    {
    outputPtr->SetLargestPossibleRegion(
      referenceImage->GetLargestPossibleRegion() );
    }
  else
    {
    typename TOutputImage::RegionType outputLargestPossibleRegion;
    outputLargestPossibleRegion.SetSize(m_Size);
    outputLargestPossibleRegion.SetIndex(m_OutputStartIndex);
    outputPtr->SetLargestPossibleRegion(outputLargestPossibleRegion);
    }

  // Set spacing and origin
  if ( m_UseReferenceImage && referenceImage )
    {
    outputPtr->SetSpacing( referenceImage->GetSpacing() );
    outputPtr->SetOrigin( referenceImage->GetOrigin() );
    outputPtr->SetDirection( referenceImage->GetDirection() );
    }
  else
    {
    outputPtr->SetSpacing(m_OutputSpacing);
    outputPtr->SetOrigin(m_OutputOrigin);
    outputPtr->SetDirection(m_OutputDirection);
    }
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
ModifiedTimeType
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::GetMTime(void) const
{
  ModifiedTimeType latestTime = Object::GetMTime();

  for (const auto& interpolator : m_Interpolators)
  {
    if (interpolator.second.GetPointer())
    {
      if (latestTime < interpolator.second->GetMTime())
      {
        latestTime = interpolator.second->GetMTime();
      }
    }
  }

  return latestTime;
}

template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType,
          typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "DefaultPixelValue: "
     << static_cast< typename NumericTraits< PixelType >::PrintType >
  ( m_DefaultPixelValue )
     << std::endl;
  os << indent << "Size: " << m_Size << std::endl;
  os << indent << "OutputStartIndex: " << m_OutputStartIndex << std::endl;
  os << indent << "OutputSpacing: " << m_OutputSpacing << std::endl;
  os << indent << "OutputOrigin: " << m_OutputOrigin << std::endl;
  os << indent << "OutputDirection: " << m_OutputDirection << std::endl;
  for (const auto& interpolator : m_Interpolators)
  {
    os << indent << "Interpolator: " << interpolator.second.GetPointer() << std::endl;
  }
  os << indent << "UseReferenceImage: " << ( m_UseReferenceImage ? "On" : "Off" )
     << std::endl;
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::EnsureTransforms()
{
  const auto inputCount = this->GetNumberOfIndexedInputs();
  for (unsigned int i = 0; i < inputCount; ++i)
  {
    auto input = this->GetInput(i);

    if (nullptr == input)
    {
      itkExceptionMacro(<< "Nth input image is not set (n: " << i << ").");
    }

    auto transform = this->GetTransform(i);
    if (nullptr == transform)
    {
      this->SetTransform(i, itk::IdentityTransform< TTransformPrecisionType, ImageDimension>::New().GetPointer());
    }
  }
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
void
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::EnsureInterpolators()
{
  const auto inputCount = this->GetNumberOfIndexedInputs();
  for (unsigned int i = 0; i < inputCount; ++i)
  {
    auto input = this->GetInput(i);

    if (nullptr == input)
    {
      itkExceptionMacro(<< "Nth input image is not set (n: " << i << ").");
    }

    if (m_Interpolators[input].IsNull())
    {
      m_Interpolators[input] = LinearInterpolatorType::New().GetPointer();
    }
  }
}

template< typename TInputImage,
  typename TOutputImage,
  typename TInterpolatorPrecisionType,
  typename TTransformPrecisionType >
std::string
StitchImageFilter< TInputImage, TOutputImage, TInterpolatorPrecisionType, TTransformPrecisionType >
::GetTransformInputName(unsigned int index)
{
  return "transform_" + std::to_string(index);
}

} // end namespace itk

#endif
