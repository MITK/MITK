/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <ipFunc/ipFunc.h>
#include <ipPic/ipPic.h>
#include <algorithm>

mitk::LevelWindow::LevelWindow(mitk::ScalarType level, mitk::ScalarType window) : m_Min(level-window/2.0), m_Max(level+window/2.0), m_RangeMin(-2048), m_RangeMax(4096), m_DefaultRangeMin(-2048), m_DefaultRangeMax(4096), m_DefaultLevel(level), m_DefaultWindow(window)
{
}

mitk::LevelWindow::LevelWindow(const mitk::LevelWindow& levWin)
{
  *this=levWin;
}

mitk::LevelWindow::~LevelWindow()
{
}

mitk::ScalarType mitk::LevelWindow::GetLevel() const
{
  return (m_Max-m_Min)/2.0 + m_Min;
}

mitk::ScalarType mitk::LevelWindow::GetWindow() const
{
  return (m_Max-m_Min);
}

mitk::ScalarType mitk::LevelWindow::GetDefaultLevel() const
{
  return m_DefaultLevel;
}

mitk::ScalarType mitk::LevelWindow::GetDefaultWindow() const
{
  return m_DefaultWindow;
}

void mitk::LevelWindow::ResetDefaultLevelWindow()
{
  SetLevelWindow(m_DefaultLevel, m_DefaultWindow);
}

mitk::ScalarType mitk::LevelWindow::GetMin() const
{
  return m_Min;
}

mitk::ScalarType mitk::LevelWindow::GetMax() const
{
  return m_Max;
}

void mitk::LevelWindow::SetDefaultLevelWindow(mitk::ScalarType level, mitk::ScalarType window)
{
  m_DefaultLevel = level;
  m_DefaultWindow = window;
}

void mitk::LevelWindow::SetLevelWindow(mitk::ScalarType level, mitk::ScalarType window)
{
  m_Min = level - window / 2.0;
  m_Max = level + window / 2.0;

  testValues();
}

void mitk::LevelWindow::SetMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if(min>max)
    std::swap(min,max);

  m_Min = min;
  m_Max = max;
  if (m_Min < m_RangeMin)
    m_Min = m_RangeMin;
  if (m_Min >= m_RangeMax)
    m_Min = m_RangeMax - 1;
  if (m_Max > m_RangeMax)
    m_Max = m_RangeMax;
  if (m_Max <= m_RangeMin)
    m_Max = m_RangeMin + 1;
  testValues();
}

void mitk::LevelWindow::SetRangeMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if(min > max)
    std::swap(min, max);
  m_RangeMin = min;
  m_RangeMax = max;
  if ( m_RangeMin == m_RangeMax)
    m_RangeMin = m_RangeMax - 1;
  if (m_Min < m_RangeMin)
    m_Min = m_RangeMin;
  if (m_Min >= m_RangeMax)
    m_Min = m_RangeMax - 1;
  if (m_Max > m_RangeMax)
    m_Max = m_RangeMax;
  if (m_Max <= m_RangeMin)
    m_Max = m_RangeMin + 1;

  testValues();
}

void mitk::LevelWindow::SetDefaultRangeMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if(min > max)
    std::swap(min, max);
  m_DefaultRangeMin = min;
  m_DefaultRangeMax = max;
  if ( m_DefaultRangeMin == m_DefaultRangeMax)
    m_DefaultRangeMin = m_DefaultRangeMax - 1;
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

mitk::ScalarType mitk::LevelWindow::GetDefaultRangeMax() const
{
  return m_DefaultRangeMax;
}

mitk::ScalarType mitk::LevelWindow::GetDefaultRangeMin() const
{
  return m_DefaultRangeMin;
}

void mitk::LevelWindow::ResetDefaultRangeMinMax()
{
  SetRangeMinMax(m_DefaultRangeMin, m_DefaultRangeMax);
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
void mitk::LevelWindow::SetAuto(mitk::Image* image, bool tryPicTags, bool guessByCentralSlice)
{
    if ( ( image == NULL ) || ( !image->IsInitialized()) )
        return;

    mitk::Image* wholeImage = image;
    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
    if ( guessByCentralSlice )
    {
        sliceSelector->SetInput(image);
        sliceSelector->SetSliceNr(image->GetDimension(2)/2);
        sliceSelector->SetTimeNr(image->GetDimension(3)/2);
        sliceSelector->SetChannelNr(image->GetDimension(4)/2);
        sliceSelector->Update();

        image = sliceSelector->GetOutput();
    }
    ScalarType minValue    = image->GetScalarValueMin();
    ScalarType maxValue    = image->GetScalarValueMaxNoRecompute();
    ScalarType min2ndValue = image->GetScalarValue2ndMinNoRecompute();
    ScalarType max2ndValue = image->GetScalarValue2ndMaxNoRecompute();
    unsigned int numPixelsInSlice = image->GetDimensions()[0];
    for ( unsigned int k=0;  k<image->GetDimension();  ++k ) numPixelsInSlice *= image->GetDimensions()[k];
    unsigned int minCount = image->GetCountOfMinValuedVoxelsNoRecompute();
    unsigned int maxCount = image->GetCountOfMaxValuedVoxelsNoRecompute();
    float minCountFraction = minCount/float(numPixelsInSlice);
    float maxCountFraction = maxCount/float(numPixelsInSlice);
    if ( minValue == maxValue )
    {
        // guessByCentralSlice seems to have failed, lets look at all data
        minValue    = wholeImage->GetScalarValueMin();                   
        maxValue    = wholeImage->GetScalarValueMaxNoRecompute();        
        min2ndValue = wholeImage->GetScalarValue2ndMinNoRecompute();  
        max2ndValue = wholeImage->GetScalarValue2ndMaxNoRecompute();  
        unsigned int numPixelsInDataset = image->GetDimensions()[0];
        for ( unsigned int k=0;  k<image->GetDimension();  ++k ) numPixelsInDataset *= image->GetDimensions()[k];
        minCount = image->GetCountOfMinValuedVoxelsNoRecompute();
        maxCount = image->GetCountOfMaxValuedVoxelsNoRecompute();
        minCountFraction = minCount/float(numPixelsInDataset);
        maxCountFraction = maxCount/float(numPixelsInDataset);
    }
    //// uniform image
    if ( minValue == maxValue )
    {
        maxValue = minValue+1;
    }

    //// binary image
    else if ( min2ndValue == maxValue )
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
        //// lots of min values -> make different from rest, but not miles away
        if ( minCountFraction > 0.2 )
        {
            ScalarType halfInnerRangeGapMinValue = min2ndValue - innerRange/2.0;
            minValue = std::max(minValue, halfInnerRangeGapMinValue);
        }
        else
        //// few min values -> focus on innerRange
        {
            minValue = min2ndValue;
        }

        //// lots of max values -> make different from rest
        if ( maxCountFraction > 0.2 )
        {
            ScalarType halfInnerRangeGapMaxValue = max2ndValue + innerRange/2.0;
            maxValue = std::min(maxValue, halfInnerRangeGapMaxValue);
        }
        else
        //// few max values -> focus on innerRange
        {
            maxValue = max2ndValue;
        }
    }



  if(guessByCentralSlice)
  {
    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();
      sliceSelector->SetInput(image);
      sliceSelector->SetSliceNr(image->GetDimension(2)/2);
      sliceSelector->SetTimeNr(image->GetDimension(3)/2);
      sliceSelector->SetChannelNr(image->GetDimension(4)/2);
      sliceSelector->Update();

    mitk::Image* slice = sliceSelector->GetOutput();
    minValue = slice->GetScalarValue2ndMin();
    maxValue = slice->GetScalarValue2ndMaxNoRecompute();
    if (minValue == maxValue)
    {
        // guessByCentralSlice seems to have failed, lets look at all data
        minValue = image->GetScalarValue2ndMin();
        maxValue = image->GetScalarValue2ndMaxNoRecompute();
    }
  }
  else
  {
    minValue = image->GetScalarValue2ndMin();
    maxValue = image->GetScalarValue2ndMaxNoRecompute();
  }

  if (minValue == maxValue)
  {
    maxValue = minValue+1;
  }
  SetRangeMinMax(minValue, maxValue);
  SetDefaultRangeMinMax(minValue, maxValue);
  SetMinMax(minValue, maxValue);
  SetDefaultLevelWindow((maxValue - minValue) / 2 + minValue, maxValue - minValue);
  if ( tryPicTags )
  {
    if ( SetAutoByPicTags(image->GetPic()) )
      return;
  }
}

bool mitk::LevelWindow::SetAutoByPicTags(const ipPicDescriptor* aPic)
{
  ipPicDescriptor* pic = const_cast<ipPicDescriptor*>(aPic);
  if ( pic == NULL )
  {
    return false;
  }
  ipPicTSV_t *tsv = ipPicQueryTag( pic, "LEVEL/WINDOW" );
  if( tsv != NULL )
  {
    double level = 0;
    double window = 0;
    #define GET_C_W( type, tsv, C, W )    \
      level = ((type *)tsv->value)[0];    \
      window = ((type *)tsv->value)[1];

    ipPicFORALL_2( GET_C_W, tsv, level, window );
    
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
    SetDefaultRangeMinMax(min, max);
    SetLevelWindow( level, window );
    SetDefaultLevelWindow(level, window);
    return true;
  }
  return false;
}

bool mitk::LevelWindow::operator==(const mitk::LevelWindow& levWin) const
{
  if ( m_RangeMin == levWin.GetRangeMin() && 
    m_RangeMax == levWin.GetRangeMax() && 
    m_Min == levWin.GetMin() && m_Max == levWin.GetMax() &&
    m_DefaultLevel == levWin.GetDefaultLevel() && m_DefaultWindow == levWin.GetDefaultWindow() &&
    m_DefaultRangeMin == levWin.GetDefaultRangeMin() && m_DefaultRangeMax == levWin.GetDefaultRangeMax()) {

      return true;
    }
  else {
    return false;
  }
}

bool mitk::LevelWindow::operator!=(const mitk::LevelWindow& levWin) const
{
  if ( m_RangeMin == levWin.GetRangeMin() && 
    m_RangeMax == levWin.GetRangeMax() && 
    m_Min == levWin.GetMin() && m_Max == levWin.GetMax() &&
    m_DefaultLevel == levWin.GetDefaultLevel() && m_DefaultWindow == levWin.GetDefaultWindow() &&
    m_DefaultRangeMin == levWin.GetDefaultRangeMin() && m_DefaultRangeMax == levWin.GetDefaultRangeMax()) {

      return false;
    }
  else {
    return true;
  }
}

mitk::LevelWindow& mitk::LevelWindow::operator=(const mitk::LevelWindow& levWin)
{
  if (this == &levWin) {
    return *this;
  }
  else {
    m_RangeMin = levWin.GetRangeMin();
    m_RangeMax = levWin.GetRangeMax();
    m_Min= levWin.GetMin();
    m_Max= levWin.GetMax();
    m_DefaultLevel = levWin.GetDefaultLevel();
    m_DefaultWindow = levWin.GetDefaultWindow();
    m_DefaultRangeMin = levWin.GetDefaultRangeMin();
    m_DefaultRangeMax = levWin.GetDefaultRangeMax();
    return *this;
  }
}
