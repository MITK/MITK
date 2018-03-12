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
#ifndef __itkEnhancedScalarImageToSizeZoneMatrixFilter_hxx
#define __itkEnhancedScalarImageToSizeZoneMatrixFilter_hxx

#include "itkEnhancedScalarImageToSizeZoneMatrixFilter.h"

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
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::EnhancedScalarImageToSizeZoneMatrixFilter() :
  m_NumberOfBinsPerAxis( itkGetStaticConstMacro( DefaultBinsPerAxis ) ),
  m_Min( NumericTraits<PixelType>::NonpositiveMin() ),
  m_Max( NumericTraits<PixelType>::max() ),
  m_MinDistance( NumericTraits<RealType>::ZeroValue() ),
  m_MaxDistance( NumericTraits<RealType>::max() ),
  m_InsidePixelValue( NumericTraits<PixelType>::OneValue() )
{
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfRequiredOutputs( 1 );

  const unsigned int measurementVectorSize = 2;

  this->ProcessObject::SetNthOutput( 0, this->MakeOutput( 0 ) );
  HistogramType *output = const_cast<HistogramType *>( this->GetOutput() );
  output->SetMeasurementVectorSize( measurementVectorSize );

  this->m_LowerBound.SetSize( measurementVectorSize );
  this->m_UpperBound.SetSize( measurementVectorSize );

  this->m_LowerBound[0] = this->m_Min;
  this->m_LowerBound[1] = this->m_MinDistance;
  this->m_UpperBound[0] = this->m_Max;
  this->m_UpperBound[1] = this->m_MaxDistance;
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetOffset( const OffsetType offset )
{
  OffsetVectorPointer offsetVector = OffsetVector::New();
  offsetVector->push_back( offset );
  this->SetOffsets( offsetVector );
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetInput( const ImageType *image )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 0, const_cast<ImageType *>( image ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::SetMaskImage( const ImageType *image )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput( 1, const_cast<ImageType *>( image ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
const TImageType *
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
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
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::GetMaskImage() const
{
  if( this->GetNumberOfInputs() < 2 )
  {
    return ITK_NULLPTR;
  }
  return static_cast<const ImageType *>( this->ProcessObject::GetInput( 1 ) );
}

template<typename TImageType, typename THistogramFrequencyContainer>
const typename EnhancedScalarImageToSizeZoneMatrixFilter<TImageType,
THistogramFrequencyContainer >::HistogramType *
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::GetOutput() const
{
  const HistogramType *output =
      static_cast<const HistogramType *>( this->ProcessObject::GetOutput( 0 ) );
  return output;
}

template<typename TImageType, typename THistogramFrequencyContainer>
typename EnhancedScalarImageToSizeZoneMatrixFilter<TImageType,
THistogramFrequencyContainer>::DataObjectPointer
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
::MakeOutput( DataObjectPointerArraySizeType itkNotUsed( idx ) )
{
  return HistogramType::New().GetPointer();
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
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

  //MITK_WARN << "InputImage casted.";

  //Cast the mask to an unsigned short image - with no respect to the incomimg maskimage
  //to prevent some non-templated itk issues
  typedef unsigned short                 LabelPixelType;
  typedef itk::Image<LabelPixelType, 3 > LabelImageType;

  typedef itk::CastImageFilter<ImageType, LabelImageType> MaskCastFilterType;
  typename MaskCastFilterType::Pointer maskCaster = MaskCastFilterType::New();
  maskCaster->SetInput(maskImage);
  maskCaster->Update();

  //MITK_WARN << "MaskImage casted.";

  //Set all values out of the mask to (m_Min + m_Max) / 2.
  typedef itk::MaskImageFilter< FloatImageType, LabelImageType, FloatImageType > MaskFilterType;
  typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput(floatImage);
  maskFilter->SetMaskImage(maskCaster->GetOutput());
  maskFilter->SetOutsideValue((m_Max + m_Min) / 2);
  maskFilter->Update();

  //MITK_WARN << "InputImage masked.";

  //Rescale intensity to match the size of the histogram
  typedef itk::Image< unsigned int, 3 >     OutputImageType;

  typedef itk::RescaleIntensityImageFilter< FloatImageType,OutputImageType> RescalerType;
  typename RescalerType::Pointer rescaler = RescalerType::New();
  //We use 0 for nans, all valid numbers will be 1 < x < size
  rescaler->SetOutputMinimum(  1  );
  rescaler->SetOutputMaximum( size[0] );
  rescaler->SetInput(maskFilter->GetOutput());
  rescaler->Update();

  typename OutputImageType::Pointer rescaled = rescaler->GetOutput();

  //MITK_WARN << "Intensities rescaled.";

  //Write back the nans because they get lost during rescaling

  int xx = inputImage->GetLargestPossibleRegion().GetSize()[0];
  int yy = inputImage->GetLargestPossibleRegion().GetSize()[1];
  int zz = inputImage->GetLargestPossibleRegion().GetSize()[2];

  for (int x = 0; x < xx; x++)
  {
    for (int y = 0; y < yy; y++)
    {
      for (int z = 0; z < zz; z++)
      {
        FloatImageType::IndexType indexF;
        indexF[0] = x;
        indexF[1] = y;
        indexF[2] = z;

        OutputImageType::IndexType indexO;
        indexO[0] = x;
        indexO[1] = y;
        indexO[2] = z;

        //Is Pixel NaN?
        if(floatImage->GetPixel(indexF) != floatImage->GetPixel(indexF))
        {
          rescaled->SetPixel(indexO,0);
        }
      }
    }
  }
  //All nans are now 0, the valid values are within [1,numberOfBins]

  /*
  OutputImageType::IndexType indexO;
  indexO[0] = 0;
  indexO[1] = 2;
  indexO[2] = 1;
  MITK_WARN << "is 0: " << rescaled->GetPixel(indexO);
  indexO[0] = 0;
  indexO[1] = 0;
  indexO[2] = 0;
  MITK_WARN << "is 1: " << rescaled->GetPixel(indexO);
  */

  PixelType distanceThreshold = 1 - mitk::eps;


  //Calculate the connected components
  typedef itk::ScalarConnectedComponentImageFilter <OutputImageType, OutputImageType, LabelImageType >
      ConnectedComponentImageFilterType;

  typename ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New ();
  connected->SetInput(rescaled);
  connected->SetMaskImage(maskCaster->GetOutput());
  connected->SetDistanceThreshold(distanceThreshold);
  connected->Update();

  /*
  indexO[0] = 0;
  indexO[1] = 2;
  indexO[2] = 1;
  MITK_WARN << "is 0: " << (connected->GetOutput())->GetPixel(indexO);
  indexO[0] = 0;
  indexO[1] = 0;
  indexO[2] = 0;
  MITK_WARN << "is 1: " << (connected->GetOutput())->GetPixel(indexO);

  MITK_WARN << "Connected components calculated.";
  */

  //Relabel the components
  typedef itk::RelabelComponentImageFilter <OutputImageType, OutputImageType > RelabelFilterType;
  typename RelabelFilterType::Pointer relabel = RelabelFilterType::New();

  typename RelabelFilterType::ObjectSizeType minSize = 1;

  relabel->SetInput(connected->GetOutput());
  relabel->SetMinimumObjectSize(minSize);
  relabel->Update();

  //MITK_WARN << "Components relabeled.";

  //Get the stats of the componentes
  typedef itk::LabelStatisticsImageFilter< FloatImageType, OutputImageType> LabelStatisticsImageFilterType;
  typename LabelStatisticsImageFilterType::Pointer labelStatisticsImageFilter =
      LabelStatisticsImageFilterType::New();
  labelStatisticsImageFilter->SetLabelInput( relabel->GetOutput() );
  labelStatisticsImageFilter->SetInput(floatImage);
  labelStatisticsImageFilter->UseHistogramsOn(); // needed to compute median
  labelStatisticsImageFilter->Update();

  /*
  std::cout << "Number of labels: "
            << labelStatisticsImageFilter->GetNumberOfLabels() << std::endl;
  std::cout << std::endl;
  */
  typedef typename LabelStatisticsImageFilterType::ValidLabelValuesContainerType ValidLabelValuesType;

  for(typename ValidLabelValuesType::const_iterator vIt = labelStatisticsImageFilter->GetValidLabelValues().begin();
      vIt != labelStatisticsImageFilter->GetValidLabelValues().end();
      ++vIt)
  {
    if ( labelStatisticsImageFilter->HasLabel(*vIt) )
    {
      LabelPixelType labelValue = *vIt;

      run[0] = labelStatisticsImageFilter->GetMean( labelValue );
      run[1] = labelStatisticsImageFilter->GetCount( labelValue );

      //Check for NaN and inf
      if(run[0] == run[0] && !std::isinf(std::abs(run[0])))
      {
        output->GetIndex( run, hIndex );
        output->IncreaseFrequencyOfIndex( hIndex, 1 );

        /*
        MITK_INFO << "Adding a region:";
        MITK_INFO << "\tmin: "
                  << labelStatisticsImageFilter->GetMinimum( labelValue );
        MITK_INFO << "\tmax: "
                  << labelStatisticsImageFilter->GetMaximum( labelValue );
        MITK_INFO << "\tmean: "
                  << labelStatisticsImageFilter->GetMean( labelValue );
        MITK_INFO << "\tcount: "
                  << labelStatisticsImageFilter->GetCount( labelValue );
        */
      }
    }
  }
}

template<typename TImageType, typename THistogramFrequencyContainer>
void
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
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
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
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
EnhancedScalarImageToSizeZoneMatrixFilter<TImageType, THistogramFrequencyContainer>
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
} // end of namespace Statistics
} // end of namespace itk

#endif
