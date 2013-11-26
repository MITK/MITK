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
    m_SphereMidpoint[i] = 0;
    }

  m_NumberOfGaussians = 0;
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
void
MultiGaussianImageSource< TOutputImage >
::SetRegionOfInterest( ItkVectorType roiMin, ItkVectorType roiMax )
{
  m_RegionOfInterestMax.operator=(roiMax);
  m_RegionOfInterestMin.operator=(roiMin);
}


template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::OutputImagePixelType
MultiGaussianImageSource< TOutputImage >
::GetMaxMeanValue() const
{
  return m_MeanValue;
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::OutputImagePixelType
MultiGaussianImageSource< TOutputImage >
::GetMaxValueInSphere() const
{
  return m_MaxValueInSphere;
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::IndexType
MultiGaussianImageSource< TOutputImage >
::GetMaxValueIndexInSphere() const
{
  return  m_MaxValueIndexInSphere;
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::OutputImagePixelType
MultiGaussianImageSource< TOutputImage >
::GetMinValueInSphere() const
{
  return m_MinValueInSphere;
}

template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::IndexType
MultiGaussianImageSource< TOutputImage >
::GetMinValueIndexInSphere() const
{
  return  m_MinValueIndexInSphere;
}
//----------------------------------------------------------------------------
template< class TOutputImage >
const typename MultiGaussianImageSource< TOutputImage >::IndexType
MultiGaussianImageSource< TOutputImage >
::GetSphereMidpoint() const
{
  return m_SphereMidpoint;
}

template< class TOutputImage >
const double
MultiGaussianImageSource< TOutputImage >
::MultiGaussianFunctionValueAtPoint(double x, double y, double z)
{
  double summand0, summand1, summand2, power, value = 0;
  // the for-loop represent  the sum of the gaussian function
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
  double valueReal;
  IndexType index;
  typedef typename TOutputImage::PixelType scalarType;
  typename TOutputImage::Pointer image = this->GetOutput(0);
  image = this->GetOutput(0);
  image->SetBufferedRegion( image->GetRequestedRegion() );
  image->Allocate();
  IteratorType imageIt(image, image->GetLargestPossibleRegion());
  PointType globalCoordinate;
  for(imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
  {
    valueReal = 0.0;
    index = imageIt.GetIndex();
    image->TransformIndexToPhysicalPoint(imageIt.GetIndex(), globalCoordinate);
    valueReal = MultiGaussianFunctionValueAtPoint(globalCoordinate[0], globalCoordinate[1] ,globalCoordinate[2]);
    imageIt.Set(valueReal);
  }
}

//----------------------------------------------------------------------------
/*
 This class allows by the method CalculateMidpointAndMeanValue() to find a sphere with a specified radius that has a maximal mean value over all      sphere with that radius with midpoint inside or at the boundary of the image. The parameter RadiusStepNumber controls the accuracy of that           calculation (the higher the value the higher the exactness).
 The algorithm works as follows:
  1. the first three for-loops traverse the region of interest and assume the current point to be the wanted sphere midpoint
  2. calculate the mean value for that sphere (use sphere coordinates):
     2.1. traverse the radius of the sphere with step size Radius divided by RadiusStepNumber (the for-loop with index i)
     2.2. define a variable dist, which gives a approximately distance between the points at the sphere surface
          (here we take such a distance, that on the smallest sphere are located 8 points)
     2.3. calculate the angles so that the points are equally spaced on the surface (for-loops with indexes j and k)
     2.3. for all radius length add the values at the points on the sphere and divide by the number of points added
          (the values at each point is calculate with the method MultiGaussianFunctionValueAtPoint())
  3. Compare with the until-now-found-maximum and take the bigger one
*/
template< typename TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::CalculateMidpointAndMeanValue()
{
  itkDebugMacro(<< "Generating a  image of scalars ");

  int numberSummand = 0, angleStepNumberOverTwo;
  double x, y, z, temp;
  MultiGaussianImageSource< TOutputImage >::OutputImagePixelType value, meanValueTemp, valueAdd;
  PointType globalCoordinate;
  IndexType index;
  double riStep, fijStep, psikStep, ri, fij, psik;
  double dist = itk::Math::pi * m_Radius / (2 * m_RadiusStepNumber);
  m_MeanValue = 0;
  riStep = m_Radius / m_RadiusStepNumber;
  OutputImageRegionType  regionOfInterest;
  IndexType indexR;
  indexR.SetElement(0, m_RegionOfInterestMin[0]);
  indexR.SetElement(1, m_RegionOfInterestMin[1]);
  indexR.SetElement(2, m_RegionOfInterestMin[2]);
  regionOfInterest.SetIndex(indexR);
  SizeType sizeROI;
  sizeROI.SetElement(0, m_RegionOfInterestMax[0] - m_RegionOfInterestMin[0] +1);
  sizeROI.SetElement(1, m_RegionOfInterestMax[1] - m_RegionOfInterestMin[1] +1);
  sizeROI.SetElement(2, m_RegionOfInterestMax[2] - m_RegionOfInterestMin[2] +1);
  regionOfInterest.SetSize(sizeROI);
  typename TOutputImage::Pointer image = this->GetOutput(0);
  IteratorType regionOfInterestIterator(image, regionOfInterest);

  for(regionOfInterestIterator.GoToBegin(); !regionOfInterestIterator.IsAtEnd(); ++regionOfInterestIterator)
  {
    numberSummand = 1;
    value = regionOfInterestIterator.Get();
    m_ValueAtMidpoint =  value;
    index = regionOfInterestIterator.GetIndex();
    image->TransformIndexToPhysicalPoint(regionOfInterestIterator.GetIndex(), globalCoordinate);
    ri = riStep;
    for(unsigned int i = 0; i < m_RadiusStepNumber; ++i)
    {
      angleStepNumberOverTwo = static_cast<int>( itk::Math::pi * ri / dist);
      fij = 0.0;
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
        fij = fij  + fijStep;
      }
      ri = ri + riStep;
    }
    meanValueTemp = value / numberSummand;
    if(meanValueTemp > m_MeanValue)
    {
      m_GlobalCoordinate = globalCoordinate;
      m_MeanValue = meanValueTemp;
      m_SphereMidpoint = index;
    }
  }
}


//-------------------------------------------------------------------------------------------------------------------------------------------
template< typename TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::OptimizeMeanValue()
{
  int radiusStepNumberOptimized = m_RadiusStepNumber * 4;
  int numberSummand = 1, angleStepNumberOverTwo;
  double x, y, z, temp;
  double riStep, fijStep, psikStep, ri, fij, psik;
  double dist = itk::Math::pi * m_Radius / (2 * radiusStepNumberOptimized);
  MultiGaussianImageSource< TOutputImage >::OutputImagePixelType valueAdd, value,
  m_MeanValue = 0;
  riStep = m_Radius / radiusStepNumberOptimized;
  ri = riStep;
  value =  m_ValueAtMidpoint;
  for(unsigned int i = 0; i < radiusStepNumberOptimized; ++i)
  {
    angleStepNumberOverTwo = static_cast<int>( itk::Math::pi * ri / dist);
    fij = 0.0;
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
        valueAdd = MultiGaussianFunctionValueAtPoint(x + m_GlobalCoordinate[0] , y + m_GlobalCoordinate[1], z + m_GlobalCoordinate[2]);
        value = value + valueAdd;
        psik = psik + psikStep;
      }
      fij = fij  + fijStep;
    }
    ri = ri + riStep;
  }
  m_MeanValue = value / numberSummand;
}


//-------------------------------------------------------------------------------------------------------------------------------------------
template< typename TOutputImage >
void
MultiGaussianImageSource< TOutputImage >
::CalculateMaxAndMinInSphere()
{
  IndexType index;
  MultiGaussianImageSource< TOutputImage >::OutputImagePixelType value;
  m_MaxValueInSphere = std::numeric_limits<OutputImagePixelType>::min();
  m_MinValueInSphere = std::numeric_limits<OutputImagePixelType>::max();
  int radInt0 = static_cast<int>(m_Radius/m_Spacing[0]);
  int radInt1 = static_cast<int>(m_Radius/m_Spacing[1]);
  int radInt2 = static_cast<int>(m_Radius/m_Spacing[2]);
  OutputImageRegionType  cuboidRegion;
  IndexType indexR;
  indexR.SetElement(0, m_SphereMidpoint[0] - radInt0);
  indexR.SetElement(1, m_SphereMidpoint[1] - radInt1);
  indexR.SetElement(2, m_SphereMidpoint[2] - radInt2);
  cuboidRegion.SetIndex(indexR);
  SizeType size;
  size.SetElement(0, 2 *radInt0 + 1);
  size.SetElement(1, 2 *radInt1 + 1);
  size.SetElement(2, 2* radInt2 + 1);
  cuboidRegion.SetSize(size);
  typename TOutputImage::Pointer image = this->GetOutput(0);
  IteratorType cuboidRegionOfInterestIterator(image, cuboidRegion);
  PointType globalCoordinate;
  for(cuboidRegionOfInterestIterator.GoToBegin(); !cuboidRegionOfInterestIterator.IsAtEnd(); ++cuboidRegionOfInterestIterator)
  {
    index = cuboidRegionOfInterestIterator.GetIndex();
    image->TransformIndexToPhysicalPoint(cuboidRegionOfInterestIterator.GetIndex(), globalCoordinate);
    if( m_GlobalCoordinate.EuclideanDistanceTo(globalCoordinate) <= m_Radius )
    {
      value = cuboidRegionOfInterestIterator.Get();
      if(m_MaxValueInSphere < value)
      {
        m_MaxValueInSphere = value;
        m_MaxValueIndexInSphere = index;
      }
      if(m_MinValueInSphere > value)
      {
        m_MinValueInSphere = value;
        m_MinValueIndexInSphere = index;
      }
    }
  }
}
} // end namespace itk

#endif
