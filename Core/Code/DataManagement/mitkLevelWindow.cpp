/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkLevelWindow.h"
#include "mitkImageSliceSelector.h"

#include <ipFunc/mitkIpFunc.h>
#include <mitkIpPic.h>
#include <algorithm>

void mitk::LevelWindow::EnsureConsistency()
{
  // Check if total range is ok
  {
    if ( m_RangeMin > m_RangeMax ) 
      std::swap(m_RangeMin,m_RangeMax);
    if (m_RangeMin == m_RangeMax ) 
      m_RangeMin = m_RangeMax - 1;
  }
  
  // Check if current window is ok
  { 
    if ( m_LowerWindowBound > m_UpperWindowBound )   
      std::swap(m_LowerWindowBound,m_UpperWindowBound);

    if ( m_LowerWindowBound < m_RangeMin ) m_LowerWindowBound = m_RangeMin;
    if ( m_UpperWindowBound < m_RangeMin ) m_UpperWindowBound = m_RangeMin;
    if ( m_LowerWindowBound > m_RangeMax ) m_LowerWindowBound = m_RangeMax;
    if ( m_UpperWindowBound > m_RangeMax ) m_UpperWindowBound = m_RangeMax;

    if (m_LowerWindowBound == m_UpperWindowBound )  
    {
      if(m_LowerWindowBound == m_RangeMin )
        m_UpperWindowBound++;
      else    
        m_LowerWindowBound--;
    }
  }
}
 
mitk::LevelWindow::LevelWindow(mitk::LevelWindow::ScalarType level, mitk::LevelWindow::ScalarType window)
: m_LowerWindowBound( level - window / 2.0 ), m_UpperWindowBound( level + window / 2.0 ),
  m_RangeMin( -2048.0 ), m_RangeMax( 4096.0 ),
  m_DefaultLowerBound( -2048.0 ), m_DefaultUpperBound( 4096.0 ),
  m_Fixed( false )
{
  SetDefaultLevelWindow(level, window);
}

mitk::LevelWindow::LevelWindow(const mitk::LevelWindow& levWin)
: m_LowerWindowBound( levWin.GetLowerWindowBound() )
, m_UpperWindowBound( levWin.GetUpperWindowBound() )
, m_RangeMin( levWin.GetRangeMin() )
, m_RangeMax( levWin.GetRangeMax() )
, m_DefaultLowerBound( levWin.GetDefaultLowerBound() )
, m_DefaultUpperBound( levWin.GetDefaultUpperBound() )
, m_Fixed( levWin.GetFixed() )
{
}

mitk::LevelWindow::~LevelWindow()
{
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetLevel() const
{
  return (m_UpperWindowBound-m_LowerWindowBound) / 2.0 + m_LowerWindowBound;
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetWindow() const
{
  return (m_UpperWindowBound-m_LowerWindowBound);
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetDefaultLevel() const
{
  return ((m_DefaultUpperBound+m_DefaultLowerBound)/2.0);
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetDefaultWindow() const
{
  return ((m_DefaultUpperBound-m_DefaultLowerBound));
}

void mitk::LevelWindow::ResetDefaultLevelWindow()
{
  SetLevelWindow(GetDefaultLevel(), GetDefaultWindow());
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetLowerWindowBound() const
{
  return m_LowerWindowBound;
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetUpperWindowBound() const
{
  return m_UpperWindowBound;
}

void mitk::LevelWindow::SetDefaultLevelWindow(mitk::LevelWindow::ScalarType level, mitk::LevelWindow::ScalarType window)
{
  SetDefaultBoundaries((level-(window/2)), (level+(window/2)));
}

void mitk::LevelWindow::SetLevelWindow(mitk::LevelWindow::ScalarType level, mitk::LevelWindow::ScalarType window)
{
  SetWindowBounds((level-(window/2.0)), (level+(window/2.0)));
}

void mitk::LevelWindow::SetWindowBounds(mitk::LevelWindow::ScalarType lowerBound, mitk::LevelWindow::ScalarType upperBound)
{
  if ( IsFixed() ) return;
  m_LowerWindowBound = lowerBound;
  m_UpperWindowBound = upperBound;
  EnsureConsistency();
}

void mitk::LevelWindow::SetRangeMinMax(mitk::LevelWindow::ScalarType min, mitk::LevelWindow::ScalarType max)
{
  if ( IsFixed() ) return;
  m_RangeMin = min;
  m_RangeMax = max;
  EnsureConsistency();
}

void mitk::LevelWindow::SetDefaultBoundaries(mitk::LevelWindow::ScalarType low, mitk::LevelWindow::ScalarType up)
{
  if ( IsFixed() )  return;
  m_DefaultLowerBound = low;
  m_DefaultUpperBound = up;
  // Check if default window is ok
  { 
    if ( m_DefaultLowerBound > m_DefaultUpperBound )   
      std::swap(m_DefaultLowerBound,m_DefaultUpperBound);

    if (m_DefaultLowerBound == m_DefaultUpperBound )  
        m_DefaultLowerBound--;
  }
  EnsureConsistency();
}

void mitk::LevelWindow::SetToMaxWindowSize()
{
  SetWindowBounds( m_RangeMin , m_RangeMax );
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetRangeMin() const
{
  return m_RangeMin;
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetRangeMax() const
{
  return m_RangeMax;
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetRange() const
{
  return  m_RangeMax - m_RangeMin;
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetDefaultUpperBound() const
{
  return m_DefaultUpperBound;
}

mitk::LevelWindow::ScalarType mitk::LevelWindow::GetDefaultLowerBound() const
{
  return m_DefaultLowerBound;
}

void mitk::LevelWindow::ResetDefaultRangeMinMax()
{
  SetRangeMinMax(m_DefaultLowerBound, m_DefaultUpperBound);
}

/*!
This method initializes a mitk::LevelWindow from an mitk::Image. The algorithm is as follows:

Default to taking the central image slice for quick analysis.

Compute the smallest (minValue), second smallest (min2ndValue), second largest (max2ndValue), and
largest (maxValue) data value by traversing the pixel values only once. In the
same scan it also computes the count of minValue values and maxValue values.
After that a basic histogram with specific information about the
extrems is complete.

If minValue == maxValue, the center slice is uniform and the above scan is repeated for
the complete image, not just one slice

Next, special cases of images with only 1, 2 or 3 distinct data values
have hand assigned level window ranges.

Next the level window is set relative to the inner range IR = lengthOf([min2ndValue, max2ndValue])

For count(minValue) > 20% the smallest values are frequent and should be
distinct from the min2ndValue and larger values (minValue may be std:min, may signify
something special) hence the lower end of the level window is set to min2ndValue - 0.5 * IR
 
For count(minValue) <= 20% the smallest values are not so important and can
blend with the next ones => min(level window) = min2ndValue

And analog for max(level window):
count(max2ndValue) > 20%:  max(level window) = max2ndValue + 0.5 * IR
count(max2ndValue) < 20%:  max(level window) = max2ndValue

In both 20%+ cases the level window bounds are clamped to the [minValue, maxValue] range

In consequence the level window maximizes contrast with minimal amount of
computation and does do useful things if the data contains std::min or
std:max values or has only 1 or 2 or 3 data values.
*/
void mitk::LevelWindow::SetAuto(const mitk::Image* image, bool tryPicTags, bool guessByCentralSlice)
{
  if ( IsFixed() )
    return;
  
  if ( image == NULL || !image->IsInitialized() ) return;

  const mitk::Image* wholeImage = image;
  ScalarType minValue = 0.0;
  ScalarType maxValue = 0.0;
  ScalarType min2ndValue = 0.0; 
  ScalarType max2ndValue = 0.0;
  mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
  if ( guessByCentralSlice )
  {
    sliceSelector->SetInput(image);
    sliceSelector->SetSliceNr(image->GetDimension(2)/2);
    sliceSelector->SetTimeNr(image->GetDimension(3)/2);
    sliceSelector->SetChannelNr(image->GetDimension(4)/2);
    sliceSelector->Update();
    image = sliceSelector->GetOutput();
    if ( image == NULL || !image->IsInitialized() ) return;

    minValue    = image->GetScalarValueMin();
    maxValue    = image->GetScalarValueMaxNoRecompute();
    min2ndValue = image->GetScalarValue2ndMinNoRecompute(); 
    max2ndValue = image->GetScalarValue2ndMaxNoRecompute();
    if ( minValue == maxValue )
    {
      // guessByCentralSlice seems to have failed, lets look at all data
      image       = wholeImage;
      minValue    = image->GetScalarValueMin();                   
      maxValue    = image->GetScalarValueMaxNoRecompute();
      min2ndValue = image->GetScalarValue2ndMinNoRecompute();
      max2ndValue = image->GetScalarValue2ndMaxNoRecompute();
    }
  }
  else
  {
    const_cast<Image*>(image)->Update();
    minValue    = image->GetScalarValueMin(0);
    maxValue    = image->GetScalarValueMaxNoRecompute(0);
    min2ndValue = image->GetScalarValue2ndMinNoRecompute(0);
    max2ndValue = image->GetScalarValue2ndMaxNoRecompute(0);
    for (unsigned int i = 1; i < image->GetDimension(3); ++i)
    {
      ScalarType minValueTemp = image->GetScalarValueMin(i);
      if (minValue > minValueTemp)
        minValue    = minValueTemp;
      ScalarType maxValueTemp = image->GetScalarValueMaxNoRecompute(i);
      if (maxValue < maxValueTemp)
        maxValue = maxValueTemp;
      ScalarType min2ndValueTemp = image->GetScalarValue2ndMinNoRecompute(i);
      if (min2ndValue > min2ndValueTemp)
        min2ndValue = min2ndValueTemp; 
      ScalarType max2ndValueTemp = image->GetScalarValue2ndMaxNoRecompute(i);
      if (max2ndValue > max2ndValueTemp)
        max2ndValue = max2ndValueTemp; 
    }
  }

  // Fix for bug# 344 Level Window wird bei Eris Cut bildern nicht richtig gesetzt
  if (image->GetPixelType().GetType() == mitkIpPicInt && image->GetPixelType().GetBpe() >= 8)
  {
    if (minValue == -(pow((double)2.0,image->GetPixelType().GetBpe())/2))
    {
      minValue = min2ndValue;
    }
  }
  // End fix

  //// uniform image
  if ( minValue == maxValue )
  {
    minValue = maxValue-1;
  }
  SetRangeMinMax(minValue, maxValue);
  SetDefaultBoundaries(minValue, maxValue);

  if ( tryPicTags ) // level and window will be set by informations provided directly by the mitkIpPicDescriptor
  {
    if ( SetAutoByPicTags(const_cast<Image*>(image)->GetPic()) )
    {
      return;
    }
  }
   
   
  unsigned int numPixelsInDataset = image->GetDimensions()[0];
  for ( unsigned int k=0;  k<image->GetDimension();  ++k ) numPixelsInDataset *= image->GetDimensions()[k];
  unsigned int minCount = image->GetCountOfMinValuedVoxelsNoRecompute();
  unsigned int maxCount = image->GetCountOfMaxValuedVoxelsNoRecompute();
  float minCountFraction = minCount/float(numPixelsInDataset);
  float maxCountFraction = maxCount/float(numPixelsInDataset);

  //// binary image
  if ( min2ndValue == maxValue )
  {
    // noop; full range is fine
  }

  //// triple value image, put middle value in center of gray level ramp
  else if ( min2ndValue == max2ndValue )
  {
    ScalarType minDelta = std::min(min2ndValue-minValue, maxValue-min2ndValue);
    minValue = min2ndValue - minDelta;
    maxValue = min2ndValue + minDelta;
  }

  // now we can assume more than three distict scalar values
  else
  {
    ScalarType innerRange = max2ndValue - min2ndValue;
    
    if ( minCountFraction > 0.2 )  //// lots of min values -> make different from rest, but not miles away
    {
      ScalarType halfInnerRangeGapMinValue = min2ndValue - innerRange/2.0;
      minValue = std::max(minValue, halfInnerRangeGapMinValue);
    }
    else  //// few min values -> focus on innerRange
    {
      minValue = min2ndValue;
    }

    if ( maxCountFraction > 0.2 )  //// lots of max values -> make different from rest
    {
      ScalarType halfInnerRangeGapMaxValue = max2ndValue + innerRange/2.0;
      maxValue = std::min(maxValue, halfInnerRangeGapMaxValue);
    }
    else  //// few max values -> focus on innerRange
    {
      maxValue = max2ndValue;
    }
  }
  SetWindowBounds(minValue, maxValue);
  SetDefaultLevelWindow((maxValue - minValue) / 2 + minValue, maxValue - minValue);
}

bool mitk::LevelWindow::SetAutoByPicTags(const mitkIpPicDescriptor* aPic)
{
  if ( IsFixed() )
    return false;
  
  mitkIpPicDescriptor* pic = const_cast<mitkIpPicDescriptor*>(aPic);
  if ( pic == NULL )
  {
    return false;
  }
  mitkIpPicTSV_t *tsv = mitkIpPicQueryTag( pic, "LEVEL/WINDOW" );
  if( tsv != NULL )
  {
    double level = 0;
    double window = 0;
    #define GET_C_W( type, tsv, C, W )    \
      level = ((type *)tsv->value)[0];    \
      window = ((type *)tsv->value)[1];

    mitkIpPicFORALL_2( GET_C_W, tsv, level, window );
    
    ScalarType min = GetRangeMin();
    ScalarType max = GetRangeMax();
    if ((double)(GetRangeMin()) > (level - window/2))
    {
      min = level - window/2;
    }
    if ((double)(GetRangeMax()) < (level + window/2))
    {
      max = level + window/2;
    }
    SetRangeMinMax(min, max);
    SetDefaultBoundaries(min, max);
    SetLevelWindow( level, window );
    return true;
  }
  return false;
}

void mitk::LevelWindow::SetFixed( bool fixed )
{
  m_Fixed = fixed; 
}
  
bool mitk::LevelWindow::GetFixed() const
{
  return m_Fixed;
}
  
bool mitk::LevelWindow::IsFixed() const
{
  return m_Fixed;
}

bool mitk::LevelWindow::operator==(const mitk::LevelWindow& levWin) const
{
  if ( m_RangeMin == levWin.GetRangeMin() && 
    m_RangeMax == levWin.GetRangeMax() && 
    m_LowerWindowBound == levWin.GetLowerWindowBound() && m_UpperWindowBound == levWin.GetUpperWindowBound() &&
    m_DefaultLowerBound == levWin.GetDefaultLowerBound() && m_DefaultUpperBound == levWin.GetDefaultUpperBound() && m_Fixed == levWin.IsFixed() ) {

      return true;
    }
  else {
    return false;
  }
}

bool mitk::LevelWindow::operator!=(const mitk::LevelWindow& levWin) const
{
  return ! ( (*this) == levWin);
}

mitk::LevelWindow& mitk::LevelWindow::operator=(const mitk::LevelWindow& levWin)
{
  if (this == &levWin) {
    return *this;
  }
  else {
    m_RangeMin = levWin.GetRangeMin();
    m_RangeMax = levWin.GetRangeMax();
    m_LowerWindowBound= levWin.GetLowerWindowBound();
    m_UpperWindowBound= levWin.GetUpperWindowBound();
    m_DefaultLowerBound = levWin.GetDefaultLowerBound();
    m_DefaultUpperBound = levWin.GetDefaultUpperBound();
    m_Fixed = levWin.GetFixed();
    return *this;
  }
}
