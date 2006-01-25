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
#include <algorithm>

mitk::LevelWindow::LevelWindow(mitk::ScalarType level, mitk::ScalarType window) : m_Min(level-window/2.0), m_Max(level+window/2.0), m_RangeMin(-1024), m_RangeMax(4096)
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

mitk::ScalarType mitk::LevelWindow::GetMin() const
{
  return m_Min;
}

mitk::ScalarType mitk::LevelWindow::GetMax() const
{
  return m_Max;
}

void mitk::LevelWindow::SetLevel(mitk::ScalarType level)
{
  mitk::ScalarType halfWindow = (m_Max - m_Min) / 2;
  m_Min = level - halfWindow;
  m_Max = level + halfWindow;

  testValues();
}

void mitk::LevelWindow::SetWindow(mitk::ScalarType window)
{
  mitk::ScalarType level = m_Min + (m_Max - m_Min) / 2;
  mitk::ScalarType halfWindow = window / 2;

  m_Min = level - halfWindow;
  m_Max = level + halfWindow;

  testValues();
}

void mitk::LevelWindow::SetLevelWindow(mitk::ScalarType level, mitk::ScalarType window)
{
  m_Min = level - window / 2.0;
  m_Max = level + window / 2.0;

  testValues();
}

void mitk::LevelWindow::SetMin(mitk::ScalarType min)
{
  m_Min = min;
  testValues(); 
}

void mitk::LevelWindow::SetMax(mitk::ScalarType max)
{
  m_Max = max;
  testValues();
}

void mitk::LevelWindow::SetMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if(min>max)
    std::swap(min,max);

  SetMin(min);
  SetMax(max);
}

void mitk::LevelWindow::SetRangeMin(mitk::ScalarType min)
{
  m_RangeMin = min;
  if(m_RangeMin > m_RangeMax)
    std::swap(m_RangeMin,m_RangeMax);
  testValues();
}

void mitk::LevelWindow::SetRangeMax(mitk::ScalarType max)
{
  m_RangeMax = max;
  if(m_RangeMin > m_RangeMax)
    std::swap(m_RangeMin,m_RangeMax);
  testValues();
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

void mitk::LevelWindow::SetAuto(mitk::Image* image, bool tryPicTags, bool guessByCentralSlice)
{
    if ( image == NULL )
        return;

    if ( tryPicTags )
    {
        if ( SetAutoByPicTags(image->GetPic()) )
            return;
    }

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
    for ( int k=0;  k<image->GetDimension();  ++k ) numPixelsInSlice *= image->GetDimensions()[k];
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
        for ( int k=0;  k<image->GetDimension();  ++k ) numPixelsInDataset *= image->GetDimensions()[k];
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
  SetRangeMin(minValue);
  SetRangeMax(maxValue);
  SetMinMax(minValue, maxValue);
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

    SetLevelWindow( level, window );
    return true;
  }
  return false;
}

bool mitk::LevelWindow::operator==(const mitk::LevelWindow& levWin) const
{
  if ( m_RangeMin == levWin.GetRangeMin() && 
    m_RangeMax == levWin.GetRangeMax() && 
    m_Min == levWin.GetMin() && m_Max == levWin.GetMax()) {

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
    m_Min == levWin.GetMin() && m_Max == levWin.GetMax()) {

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
    return *this;
  }
}
