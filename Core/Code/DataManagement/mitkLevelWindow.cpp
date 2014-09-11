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


#include "mitkLevelWindow.h"
#include "mitkImage.h"
#include "mitkImageSliceSelector.h"
#include "mitkImageStatisticsHolder.h"

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

mitk::LevelWindow::LevelWindow(mitk::ScalarType level, mitk::ScalarType window)
: m_LowerWindowBound( level - window / 2.0 ), m_UpperWindowBound( level + window / 2.0 ),
  m_RangeMin( -2048.0 ), m_RangeMax( 4096.0 ),
  m_DefaultLowerBound( -2048.0 ), m_DefaultUpperBound( 4096.0 ),
  m_Fixed( false )
{
  SetDefaultLevelWindow(level, window);
  SetLevelWindow(level, window, true);
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

mitk::ScalarType mitk::LevelWindow::GetLevel() const
{
  return (m_UpperWindowBound-m_LowerWindowBound) / 2.0 + m_LowerWindowBound;
}

mitk::ScalarType mitk::LevelWindow::GetWindow() const
{
  return (m_UpperWindowBound-m_LowerWindowBound);
}

mitk::ScalarType mitk::LevelWindow::GetDefaultLevel() const
{
  return ((m_DefaultUpperBound+m_DefaultLowerBound)/2.0);
}

mitk::ScalarType mitk::LevelWindow::GetDefaultWindow() const
{
  return ((m_DefaultUpperBound-m_DefaultLowerBound));
}

void mitk::LevelWindow::ResetDefaultLevelWindow()
{
  SetLevelWindow(GetDefaultLevel(), GetDefaultWindow());
}

mitk::ScalarType mitk::LevelWindow::GetLowerWindowBound() const
{
  return m_LowerWindowBound;
}

mitk::ScalarType mitk::LevelWindow::GetUpperWindowBound() const
{
  return m_UpperWindowBound;
}

void mitk::LevelWindow::SetDefaultLevelWindow(mitk::ScalarType level, mitk::ScalarType window)
{
  SetDefaultBoundaries((level-(window/2.0)), (level+(window/2.0)));
}

void mitk::LevelWindow::SetLevelWindow(mitk::ScalarType level, mitk::ScalarType window, bool expandRangesIfNecessary)
{
  SetWindowBounds( (level-(window/2.0)), (level+(window/2.0)), expandRangesIfNecessary );
}

void mitk::LevelWindow::SetWindowBounds(mitk::ScalarType lowerBound, mitk::ScalarType upperBound, bool expandRangesIfNecessary)
{
  if ( IsFixed() ) return;

  m_LowerWindowBound = lowerBound;
  m_UpperWindowBound = upperBound;

  if (expandRangesIfNecessary)
  {
    /* if caller is sure he wants exactly that level/window, we make sure the limits match */
    if (m_LowerWindowBound > m_UpperWindowBound)
      std::swap(m_LowerWindowBound, m_UpperWindowBound);
    if ( m_LowerWindowBound < m_RangeMin )
    {
      m_RangeMin = m_LowerWindowBound;
    }

    if ( m_UpperWindowBound > m_RangeMax )
    {
      m_RangeMax = m_UpperWindowBound;
    }
  }

  EnsureConsistency();
}

void mitk::LevelWindow::SetRangeMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if ( IsFixed() ) return;
  m_RangeMin = min;
  m_RangeMax = max;
  EnsureConsistency();
}

void mitk::LevelWindow::SetDefaultBoundaries(mitk::ScalarType low, mitk::ScalarType up)
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

mitk::ScalarType mitk::LevelWindow::GetRangeMin() const
{
  return m_RangeMin;
}

mitk::ScalarType mitk::LevelWindow::GetRangeMax() const
{
  return m_RangeMax;
}

mitk::ScalarType mitk::LevelWindow::GetRange() const
{
  return  m_RangeMax - m_RangeMin;
}

mitk::ScalarType mitk::LevelWindow::GetDefaultUpperBound() const
{
  return m_DefaultUpperBound;
}

mitk::ScalarType mitk::LevelWindow::GetDefaultLowerBound() const
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
void mitk::LevelWindow::SetAuto(const mitk::Image* image, bool /*tryPicTags*/, bool guessByCentralSlice)
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

    minValue    = image->GetStatistics()->GetScalarValueMin();
    maxValue    = image->GetStatistics()->GetScalarValueMaxNoRecompute();
    min2ndValue = image->GetStatistics()->GetScalarValue2ndMinNoRecompute();
    max2ndValue = image->GetStatistics()->GetScalarValue2ndMaxNoRecompute();
    if ( minValue == maxValue )
    {
      // guessByCentralSlice seems to have failed, lets look at all data
      image       = wholeImage;
      minValue    = image->GetStatistics()->GetScalarValueMin();
      maxValue    = image->GetStatistics()->GetScalarValueMaxNoRecompute();
      min2ndValue = image->GetStatistics()->GetScalarValue2ndMinNoRecompute();
      max2ndValue = image->GetStatistics()->GetScalarValue2ndMaxNoRecompute();
    }
  }
  else
  {
    const_cast<Image*>(image)->Update();
    minValue    = image->GetStatistics()->GetScalarValueMin(0);
    maxValue    = image->GetStatistics()->GetScalarValueMaxNoRecompute(0);
    min2ndValue = image->GetStatistics()->GetScalarValue2ndMinNoRecompute(0);
    max2ndValue = image->GetStatistics()->GetScalarValue2ndMaxNoRecompute(0);
    for (unsigned int i = 1; i < image->GetDimension(3); ++i)
    {
      ScalarType minValueTemp = image->GetStatistics()->GetScalarValueMin(i);
      if (minValue > minValueTemp)
        minValue    = minValueTemp;
      ScalarType maxValueTemp = image->GetStatistics()->GetScalarValueMaxNoRecompute(i);
      if (maxValue < maxValueTemp)
        maxValue = maxValueTemp;
      ScalarType min2ndValueTemp = image->GetStatistics()->GetScalarValue2ndMinNoRecompute(i);
      if (min2ndValue > min2ndValueTemp)
        min2ndValue = min2ndValueTemp;
      ScalarType max2ndValueTemp = image->GetStatistics()->GetScalarValue2ndMaxNoRecompute(i);
      if (max2ndValue > max2ndValueTemp)
        max2ndValue = max2ndValueTemp;
    }
  }

  // Fix for bug# 344 Level Window wird bei Eris Cut bildern nicht richtig gesetzt
  if (   image->GetPixelType().GetPixelType()==itk::ImageIOBase::SCALAR
      && image->GetPixelType().GetComponentType() == itk::ImageIOBase::INT
      && image->GetPixelType().GetBpe() >= 8)
  {
    // the windows compiler complains about ambiguos 'pow' call, therefore static casting to (double, int)
    if (minValue == -( pow( (double) 2.0, static_cast<int>(image->GetPixelType().GetBpe()/2) ) ) )
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
  else
  {
      //Due to bug #8690 level window now is no longer of fixed range by default but the range adapts according to levelwindow interaction
      //This is done because the range should be a little bit larger from the beginning so that the scale doesn't start to resize right from the beginning
      double additionalRange = 0.15*(maxValue-minValue);
      minValue -= additionalRange;
      maxValue += additionalRange;
  }
  SetRangeMinMax(minValue, maxValue);
  SetDefaultBoundaries(minValue, maxValue);
/*
  if ( tryPicTags ) // level and window will be set by informations provided directly by the mitkIpPicDescriptor
  {
    if ( SetAutoByPicTags(const_cast<Image*>(image)->GetPic()) )
    {
      return;
    }
  }
 */

  unsigned int numPixelsInDataset = image->GetDimensions()[0];
  for ( unsigned int k=0;  k<image->GetDimension();  ++k ) numPixelsInDataset *= image->GetDimensions()[k];
  unsigned int minCount = image->GetStatistics()->GetCountOfMinValuedVoxelsNoRecompute();
  unsigned int maxCount = image->GetStatistics()->GetCountOfMaxValuedVoxelsNoRecompute();
  ScalarType minCountFraction = minCount/ScalarType(numPixelsInDataset);
  ScalarType maxCountFraction = maxCount/ScalarType(numPixelsInDataset);

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
