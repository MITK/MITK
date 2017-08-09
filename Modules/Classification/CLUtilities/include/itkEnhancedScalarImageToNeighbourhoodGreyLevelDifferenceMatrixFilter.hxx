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

/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#ifndef __itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter_hxx
#define __itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter_hxx

#include "itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhood.h"
#include "vnl/vnl_math.h"
#include "itkMacro.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkLabelStatisticsImageFilter.h"
#include "itkScalarConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkCastImageFilter.h"

#include <mitkLogMacros.h>

namespace itk
{
namespace Statistics
{
template<typename TImageType, typename THistogramFrequencyContainer>
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter() :
  m_NumberOfBinsPerAxis( itkGetStaticConstMacro( DefaultBinsPerAxis ) ),
  m_Min( NumericTraits<PixelType>::NonpositiveMin() ),
  m_Max( NumericTraits<PixelType>::max() ),
  m_MinDistance( NumericTraits<RealType>::ZeroValue() ),
  m_MaxDistance( NumericTraits<RealType>::max() ),
  m_InsidePixelValue( NumericTraits<PixelType>::OneValue() )
{
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfRequiredOutputs( 1 );

  const unsigned int measurementVectorSize = 1;

  this->ProcessObject::SetNthOutput( 0, this->MakeOutput( 0 ) );
  this->ProcessObject::SetNthOutput( 1, this->MakeOutput( 1 ) );
  HistogramType *output = const_cast<HistogramType *>( this->GetOutput() );
  output->SetMeasurementVectorSize( measurementVectorSize );

  m_siMatrix = new double[m_NumberOfBinsPerAxis];
  for(unsigned int i = 0; i < m_NumberOfBinsPerAxis; i++)
  {
    m_siMatrix[i] = 0;
  }

  this->m_LowerBound.SetSize( measurementVectorSize );
  this->m_UpperBound.SetSize( measurementVectorSize );

  this->m_LowerBound[0] = this->m_Min;
  this->m_LowerBound[1] = this->m_MinDistance;
  this->m_UpperBound[0] = this->m_Max;
  this->m_UpperBound[1] = this->m_MaxDistance;
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetOffset( const OffsetType offset )
{
  OffsetVectorPointer offsetVector = OffsetVector::New();
  offsetVector->push_back( offset );
  this->SetOffsets( offsetVector );
  MITK_WARN << "We now have " << this->GetOffsets()->size() << " offsets in matrixgenerator";
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::AddOffsets( const std::vector<OffsetType> _offsets )
{
  OffsetVectorPointer offsetVector = OffsetVector::New();
  typename OffsetVector::ConstIterator offsets;
  //MITK_WARN << "We have " << this->GetOffsets()->size() << " offsets!";
  for( std::size_t i = 0; i < _offsets.size(); i++)
  {
  offsetVector->push_back(_offsets[i]);
  auto k = _offsets[i];
  this->NormalizeOffsetDirection(k);
  offsetVector->push_back(k);
  }
  this->SetOffsets( offsetVector );
  MITK_WARN << "We now have " << this->GetOffsets()->size() << " offsets in matrixgenerator";
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetInput( const ImageType *image )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 0, const_cast<ImageType *>( image ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetMaskImage( const ImageType *image )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 1, const_cast<ImageType *>( image ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
const TImageType *
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::GetInput() const
{
  if( this->GetNumberOfInputs() < 1 )
  {
    return ITK_NULLPTR;
  }
  return static_cast<const ImageType *>( this->ProcessObject::GetInput( 0 ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
const TImageType *
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::GetMaskImage() const
{
  if( this->GetNumberOfInputs() < 2 )
  {
    return ITK_NULLPTR;
  }
  return static_cast<const ImageType *>( this->ProcessObject::GetInput( 1 ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
const typename EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType,
THistogramFrequencyContainer >::HistogramType *
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::GetOutput() const
{
  const HistogramType *output =
      static_cast<const HistogramType *>( this->ProcessObject::GetOutput( 0 ) );
  return output;
}


template<typename TImageType, typename THistogramFrequencyContainer>
double* EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::GetSiMatrix() const
{
  return m_siMatrix;
}

template<typename TImageType, typename THistogramFrequencyContainer>
typename EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType,
THistogramFrequencyContainer>::DataObjectPointer
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::MakeOutput( DataObjectPointerArraySizeType itkNotUsed( idx ) )
{
  return HistogramType::New().GetPointer();
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::GenerateData()
{
  HistogramType *output =
      static_cast<HistogramType *>( this->ProcessObject::GetOutput( 0 ) );

  const ImageType * inputImage = this->GetInput();
  const ImageType * maskImage = this->GetMaskImage();

  // First, create an appropriate histogram with the right number of bins
  // and mins and maxes correct for the image type.
  typename HistogramType::SizeType size( output->GetMeasurementVectorSize() );

  size.Fill( this->m_NumberOfBinsPerAxis );
  this->m_LowerBound[0] = this->m_Min;
  this->m_LowerBound[1] = this->m_MinDistance;
  this->m_UpperBound[0] = this->m_Max;
  this->m_UpperBound[1] = this->m_MaxDistance;

  output->Initialize( size, this->m_LowerBound, this->m_UpperBound );

  MeasurementVectorType run( output->GetMeasurementVectorSize() );
  typename HistogramType::IndexType hIndex;

  //Cast the image to a float image - with no respect to the incoming image
  //to prevent some non-templated itk issues
  typedef itk::Image<float, 3> FloatImageType;
  typedef itk::CastImageFilter<ImageType, FloatImageType> CastFilterType;

  typename CastFilterType::Pointer caster = CastFilterType::New();
  caster->SetInput(inputImage);
  caster->Update();
  typename FloatImageType::Pointer floatImage = caster->GetOutput();

  //Cast the mask to an unsigned short image - with no respect to the incomimg maskimage
  //to prevent some non-templated itk issues
  typedef unsigned short                 LabelPixelType;
  typedef itk::Image<LabelPixelType, 3 > LabelImageType;

  typedef itk::CastImageFilter<ImageType, LabelImageType> MaskCastFilterType;
  typename MaskCastFilterType::Pointer maskCaster = MaskCastFilterType::New();
  maskCaster->SetInput(maskImage);
  maskCaster->Update();

  //Set all values out of the mask to nans.
  typedef itk::MaskImageFilter< FloatImageType, LabelImageType, FloatImageType > MaskFilterType;
  typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(floatImage);
  maskFilter->SetMaskImage(maskCaster->GetOutput());
  maskFilter->SetOutsideValue( std::numeric_limits<float>::quiet_NaN());
  maskFilter->Update();
  FloatImageType::Pointer floatImageMasked = maskFilter->GetOutput();

  typedef ConstNeighborhoodIterator<ImageType> NeighborhoodIteratorType;
  typename NeighborhoodIteratorType::RadiusType radius;
  radius.Fill( 1 );
  NeighborhoodIteratorType neighborIt( radius,
                                       inputImage, inputImage->GetRequestedRegion() );

  for( neighborIt.GoToBegin(); !neighborIt.IsAtEnd(); ++neighborIt )
  {
    const PixelType centerPixelIntensity = neighborIt.GetCenterPixel();
    IndexType centerIndex = neighborIt.GetIndex();

    FloatImageType::IndexType cIndex;
    cIndex[0] = centerIndex[0];
    cIndex[1] = centerIndex[1];
    cIndex[2] = centerIndex[2];
    float centerFloatPixel = floatImageMasked->GetPixel(cIndex);

    int px = 0;
    PixelType sum = 0.0;
    bool canCalculate = true;

    typename OffsetVector::ConstIterator offsets;
    for( offsets = this->GetOffsets()->Begin();
         offsets != this->GetOffsets()->End(); offsets++ )
    {
      OffsetType offset = offsets.Value();
      IndexType index;

      index = centerIndex + offset;

      if(!inputImage->GetRequestedRegion().IsInside(index))
      {
        canCalculate = false;
        break;
      }

      PixelType offsetPixel = inputImage->GetPixel(index);

      FloatImageType::IndexType fIndex;
      fIndex[0] = index[0];
      fIndex[1] = index[1];
      fIndex[2] = index[2];

      float floatPixel = floatImageMasked->GetPixel(fIndex);

      //We have a nan here
      if(floatPixel != floatPixel || centerFloatPixel!= centerFloatPixel)
      {
        canCalculate = false;
        break;
      }

      sum += offsetPixel;
      px++;
    }
    //If we have a nan in the neighbourhood, continue
    if(!canCalculate)
      continue;

    PixelType mean = sum / px;

    double si = std::abs<double>(mean-centerPixelIntensity);

    run[0] = centerPixelIntensity;

    //Check for NaN and inf
    if(run[0] == run[0] && !std::isinf(std::abs(run[0])))
    {
      output->GetIndex( run, hIndex );
      output->IncreaseFrequencyOfIndex( hIndex, 1 );

      m_siMatrix[hIndex[0]] += si;
    }
    //MITK_WARN << " -> In this round we added: " << centerIndex << " with value " << centerPixelIntensity << " and si = " << si;
    //MITK_WARN << " -> Values are now siMatrix["<<hIndex[0]<<"]: " << m_siMatrix[hIndex[0]];
    //MITK_WARN << " -> Values are now niMatrix: " << output->GetFrequency(hIndex) << "/" << run;
  }
}


template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetPixelValueMinMax( PixelType min, PixelType max )
{
  if( this->m_Min != min || this->m_Max != max )
  {
    itkDebugMacro( "setting Min to " << min << "and Max to " << max );
    this->m_Min = min;
    this->m_Max = max;
    this->Modified();
  }
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetDistanceValueMinMax( RealType min, RealType max )
{
  if( this->m_MinDistance != min || this->m_MaxDistance != max )
  {
    itkDebugMacro( "setting MinDistance to " << min << "and MaxDistance to "
                   << max );
    this->m_MinDistance = min;
    this->m_MaxDistance = max;
    this->Modified();
  }
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os,indent );
  os << indent << "Offsets: " << this->GetOffsets() << std::endl;
  os << indent << "Min: " << this->m_Min << std::endl;
  os << indent << "Max: " << this->m_Max << std::endl;
  os << indent << "Min distance: " << this->m_MinDistance << std::endl;
  os << indent << "Max distance: " << this->m_MaxDistance << std::endl;
  os << indent << "NumberOfBinsPerAxis: " << this->m_NumberOfBinsPerAxis
     << std::endl;
  os << indent << "InsidePixelValue: " << this->m_InsidePixelValue << std::endl;
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter<TImageType, THistogramFrequencyContainer>
::NormalizeOffsetDirection(OffsetType &offset)
{
  //MITK_WARN <<" -> NGTDM old offset = " << offset;
  itkDebugMacro("old offset = " << offset << std::endl);
  int sign = 1;
  bool metLastNonZero = false;
  for (int i = offset.GetOffsetDimension()-1; i>=0; i--)
  {
    if (metLastNonZero)
    {
      offset[i] *= sign;
    }
    else if (offset[i] != 0)
    {
      sign = (offset[i] > 0 ) ? 1 : -1;
      metLastNonZero = true;
      offset[i] *= sign;
    }
  }

  //MITK_WARN << " ->NGTDM new  offset = " << offset;
  itkDebugMacro("new  offset = " << offset << std::endl);
}
} // end of namespace Statistics
} // end of namespace itk

#endif
