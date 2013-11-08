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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef __itkMultiGaussianImageSource_hxx
#define __itkMultiGaussianImageSource_hxx


#include <iostream>
#include <fstream>
#include <time.h>
#include "itkMultiGaussianImageSource.h"
#include "itkImageRegionIterator.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"
#include "stdlib.h"
namespace itk
{
/**
 *
 */
template< class TOutputImage >
MultiGaussianImageSource< TOutputImage >
::MultiGaussianImageSource()
{
  //Initial image is 100 wide in each direction.
  for ( unsigned int i = 0; i < TOutputImage::GetImageDimension(); i++ )
    {
    m_Size[i] = 100;
    m_Spacing[i] = 1.0;
    m_Origin[i] = 0.0;
    m_ShpereMidpoint[i] = 0;
    }

  m_NumberOfGaussians = 1;
  m_Radius = 1;
  m_RadiusStepNumber = 5;
  m_MeanValue = 0;

  m_Min = NumericTraits< OutputImagePixelType >::NonpositiveMin();
  m_Max = NumericTraits< OutputImagePixelType >::max();
}

template< class TOutputImage >
MultiGaussianImageSource< TOutputImage >
::~MultiGaussianImageSource()
{}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::SetSize(SizeValueArrayType sizeArray)
{
  const unsigned int count = TOutputImage::ImageDimension;
  unsigned int       i;

  for ( i = 0; i < count; i++ )
    {
    if ( sizeArray[i] != this->m_Size[i] )
      {
      break;
      }
    }
  if ( i < count )
    {
    this->Modified();
    for ( i = 0; i < count; i++ )
      {
      this->m_Size[i] = sizeArray[i];
      }
    }
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::SizeValueType *
MultiGaussianImageSource< TOutputImage >
::GetSize() const
{
  return this->m_Size.GetSize();
}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::SetSpacing(SpacingValueArrayType spacingArray)
{
  const unsigned int count = TOutputImage::ImageDimension;
  unsigned int       i;

  for ( i = 0; i < count; i++ )
    {
    if ( spacingArray[i] != this->m_Spacing[i] )
      {
      break;
      }
    }
  if ( i < count )
    {
    this->Modified();
    for ( i = 0; i < count; i++ )
      {
      this->m_Spacing[i] = spacingArray[i];
      }
    }
}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::SetOrigin(PointValueArrayType originArray)
{
  const unsigned int count = TOutputImage::ImageDimension;
  unsigned int       i;

  for ( i = 0; i < count; i++ )
    {
    if ( originArray[i] != this->m_Origin[i] )
      {
      break;
      }
    }
  if ( i < count )
    {
    this->Modified();
    for ( i = 0; i < count; i++ )
      {
      this->m_Origin[i] = originArray[i];
      }
    }
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::PointValueType *
MultiGaussianImageSource< TOutputImage >
::GetOrigin() const
{
  for ( unsigned int i = 0; i < TOutputImage::ImageDimension; i++ )
    {
    this->m_OriginArray[i] = this->m_Origin[i];
    }
  return this->m_OriginArray;
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::SpacingValueType *
MultiGaussianImageSource< TOutputImage >
::GetSpacing() const
{
  for ( unsigned int i = 0; i < TOutputImage::ImageDimension; i++ )
    {
    this->m_SpacingArray[i] = this->m_Spacing[i];
    }
  return this->m_SpacingArray;
}

/**
 *
 */
template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Max: "
     << static_cast< typename NumericTraits< OutputImagePixelType >::PrintType >( m_Max )
     << std::endl;
  os << indent << "Min: "
     << static_cast< typename NumericTraits< OutputImagePixelType >::PrintType >( m_Min )
     << std::endl;

  os << indent << "Origin: [";
  unsigned int ii = 0;
  while( ii < TOutputImage::ImageDimension - 1 )
    {
    os << m_Origin[ii] << ", ";
    ++ii;
    }
  os << m_Origin[ii] << "]" << std::endl;

  os << indent << "Spacing: [";
  ii = 0;
  while( ii < TOutputImage::ImageDimension - 1 )
    {
    os << m_Spacing[ii] << ", ";
    ++ii;
    }
  os << m_Spacing[ii] << "]" << std::endl;

  os << indent << "Size: [";
  ii = 0;
  while( ii < TOutputImage::ImageDimension - 1 )
    {
    os << m_Size[ii] << ", ";
    ++ii;
    }
  os << m_Size[ii] << "]" << std::endl;
}

template< class TOutputImage >
unsigned int
MultiGaussianImageSource< TOutputImage >
::GetNumberOfGaussians() const
{
  return this->m_NumberOfGaussians;
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::RadiusType
MultiGaussianImageSource< TOutputImage >
::GetRadius() const
{
  return this->m_Radius;
}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::SetRadius( RadiusType  radius )
{
  this->m_Radius = radius;
}

template< class TOutputImage >
const int
MultiGaussianImageSource< TOutputImage >
::GetRadiusStepNumber() const
{
  return this->m_RadiusStepNumber;
}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::SetRadiusStepNumber( unsigned int stepNumber )
{
  this->m_RadiusStepNumber = stepNumber;
}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::SetNumberOfGausssians( unsigned int n )
{
  this->m_NumberOfGaussians = n;
}

template< class TOutputImage >
const double
MultiGaussianImageSource< TOutputImage >
::GetMaxMeanValue() const
{
  return m_MeanValue;
}


//----------------------------------------------------------------------------
template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::IndexType
MultiGaussianImageSource< TOutputImage >
::GetSphereMidpoint() const
{
  return m_ShpereMidpoint;
}

template< class TOutputImage >
const double
MultiGaussianImageSource< TOutputImage >
::MultiGaussianFunctionValueAtPoint(double x, double y, double z)
{
  double summand0, summand1, summand2, power, value = 0;
  for(unsigned int n =0; n < m_NumberOfGaussians; ++n)
  {
    summand0 = (x - m_CenterX[n]) / m_SigmaX[n];
    summand1 = (y - m_CenterY[n]) / m_SigmaY[n];
    summand2 = (z - m_CenterZ[n]) / m_SigmaZ[n];

    power = summand0 * summand0 + summand1 * summand1 + summand2 * summand2;
    value = value + m_Altitude[n] * pow(itk::Math::e, -0.5 * power);
  }
  return value;
}

template< class TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::AddGaussian( VectorType x, VectorType y, VectorType z, VectorType sx, VectorType sy, VectorType sz, VectorType altitude)
{
  for(unsigned int i = 0; i < x.size();  ++i)
  {
    m_CenterX.push_back(x[i]);
    m_CenterY.push_back(y[i]);
    m_CenterZ.push_back(z[i]);
    m_SigmaX.push_back(sx[i]);
    m_SigmaY.push_back(sy[i]);
    m_SigmaZ.push_back(sz[i]);
    m_Altitude.push_back(altitude[i]);
  }
}

//----------------------------------------------------------------------------
template< typename TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::GenerateOutputInformation()
{
  TOutputImage *output;
  IndexType     index;

  index.Fill(0);

  output = this->GetOutput(0);

  typename TOutputImage::RegionType largestPossibleRegion;
  largestPossibleRegion.SetSize(this->m_Size);
  largestPossibleRegion.SetIndex(index);
  output->SetLargestPossibleRegion(largestPossibleRegion);

  output->SetSpacing(m_Spacing);
  output->SetOrigin(m_Origin);
}

//----------------------------------------------------------------------------
template< typename TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::GenerateData()
{
  itkDebugMacro(<< "Generating a  image of scalars ");
  double valueReal, value;
  IndexType index;
  // Support progress methods/callbacks
  //ProgressReporter progress( this, threadId, outputRegionForThread.GetNumberOfPixels() );

  // typedef typename TOutputImage::PixelType scalarType;
  //typename TOutputImage::Pointer image = this->GetOutput(0);
  m_Image = this->GetOutput(0);
  m_Image->SetBufferedRegion( m_Image->GetRequestedRegion() );
  m_Image->Allocate();
  IteratorType imageIt(m_Image, m_Image->GetLargestPossibleRegion());
  PointType globalCoordinate;

  for(imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
  {
    valueReal = 0.0;
    index = imageIt.GetIndex();
    m_Image->TransformIndexToPhysicalPoint(imageIt.GetIndex(), globalCoordinate);
    valueReal = MultiGaussianFunctionValueAtPoint(globalCoordinate[0], globalCoordinate[1] ,globalCoordinate[2]);
    imageIt.Set(valueReal);
    //progress.CompletedPixel();
  }
}

//----------------------------------------------------------------------------
template< typename TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::CalculateMidpointAndMeanValue()
{
  itkDebugMacro(<< "Generating a  image of scalars ");
  int numberSummand = 0, angleStepNumberOverTwo;
  double valueReal, meanValueTemp, valueAdd, value, x, y, z, temp;
  double riStep, fijStep, psikStep, ri, fij, psik;
  double dist = itk::Math::pi * m_Radius / (2 * m_RadiusStepNumber);
  m_MeanValue = 0;
  riStep = m_Radius / m_RadiusStepNumber;
  IteratorType imageIt(m_Image, m_Image->GetLargestPossibleRegion());
  PointType globalCoordinate;
  IndexType  index;

  for(imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
  {
    numberSummand = 1;
    index = imageIt.GetIndex();
    m_Image->TransformIndexToPhysicalPoint(imageIt.GetIndex(), globalCoordinate);
    value = imageIt.Value();
    ri = riStep;
    for(unsigned int i = 0; i < m_RadiusStepNumber; ++i)
    {
      angleStepNumberOverTwo = static_cast<int>( itk::Math::pi * ri / dist);
      fij = 0;
      fijStep = itk::Math::pi / angleStepNumberOverTwo;
      for(unsigned int j = 0; j <= angleStepNumberOverTwo; ++j) // from 0 to pi
      {
        z = ri * cos(fij);
        psikStep = 2.0 * itk::Math::pi / (2.0 * angleStepNumberOverTwo);
        psik =  -itk::Math::pi + psikStep;

        temp =  ri * sin(fij);
        for(unsigned int k = 0; k < 2 * angleStepNumberOverTwo; ++k) // from -pi to pi
        {
          x = temp * cos(psik);
          y = temp * sin(psik);
          numberSummand++;
          valueAdd = MultiGaussianFunctionValueAtPoint(x + globalCoordinate[0], y + globalCoordinate[1], z + globalCoordinate[2]);
          value = value + valueAdd;

          psik = psik + psikStep;
        }
        fij = fij  + fijStep ;
      }
      ri = ri + riStep;
    }

    meanValueTemp = value / numberSummand;
    if(meanValueTemp > m_MeanValue)
    {
      m_MeanValue   = meanValueTemp;
      m_ShpereMidpoint = index;
    }
  }
}

} // end namespace itk
#endif
