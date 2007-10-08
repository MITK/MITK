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
#include "mitkImage.h"

#include <ipFunc/ipFunc.h>
#include <ipPic/ipPic.h>
#include <algorithm>

mitk::LevelWindow::LevelWindow(mitk::ScalarType level, mitk::ScalarType window)
: m_Min( level - window / 2.0 ), m_Max( level + window / 2.0 ),
  m_RangeMin( -2048.0 ), m_RangeMax( 4096.0 ),
  m_DefaultRangeMin( -2048.0 ), m_DefaultRangeMax( 4096.0 ),
  m_DefaultLevel( level ), m_DefaultWindow( window ),
  m_Fixed( false )
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
  return (m_Max-m_Min) / 2.0 + m_Min;
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
  if ( IsFixed() )
    return;
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
  if ( IsFixed() )
    return;
  
  m_DefaultLevel = level;
  m_DefaultWindow = window;
}

void mitk::LevelWindow::SetLevelWindow(mitk::ScalarType level, mitk::ScalarType window)
{
  if ( IsFixed() )
    return;
  
  m_Min = level - window / 2.0;
  m_Max = level + window / 2.0;

  testValues();
}

void mitk::LevelWindow::SetMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if ( IsFixed() )
    return;
  
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

void mitk::LevelWindow::SetToMaxWindowSize()
{
 m_Max = m_RangeMax;
 m_Min = m_RangeMin;
}

void mitk::LevelWindow::SetRangeMinMax(mitk::ScalarType min, mitk::ScalarType max)
{
  if ( IsFixed() )
    return;
  
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
  if ( IsFixed() )
    return;
  
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

Gets the smallest (minValue), second smallest (min2ndValue) and
largest (maxValue) data value from mitk::Image. This is done in mitk::Image by traversing the pixel values only once.

If minValue == maxValue, the image is uniform and the minValue will be set to maxValue -1.
*/
void mitk::LevelWindow::SetAuto(const mitk::Image* image, bool tryPicTags)
{
  if ( IsFixed() )
    return;
  
  if ( image == NULL || !image->IsInitialized() ) return;

  ScalarType minValue    = image->GetScalarValueMin();
  ScalarType maxValue    = image->GetScalarValueMaxNoRecompute();

  if ( tryPicTags )
  {
    if ( SetAutoByPicTags(const_cast<Image*>(image)->GetPic()) )
    {
      SetRangeMinMax(minValue, maxValue);
  SetDefaultRangeMinMax(minValue, maxValue);
      return;
    }
  }

  
  ScalarType min2ndValue = image->GetScalarValue2ndMinNoRecompute();

  // Fix for bug# 344 Level Window wird bei Eris Cut bildern nicht richtig gesetzt
  if (image->GetPixelType().GetType() == ipPicInt && image->GetPixelType().GetBpe() >= 8)
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
  SetDefaultRangeMinMax(minValue, maxValue);
  SetMinMax(minValue, maxValue);
  SetDefaultLevelWindow((maxValue - minValue) / 2 + minValue, maxValue - minValue);
}

bool mitk::LevelWindow::SetAutoByPicTags(const ipPicDescriptor* aPic)
{
  if ( IsFixed() )
    return false;
  
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
    }/*
    SetRangeMinMax(min, max);
    SetDefaultRangeMinMax(min, max);*/
    SetLevelWindow( level, window );
    SetDefaultLevelWindow(level, window);
    return true;
  }
  return false;
}

void mitk::LevelWindow::SetFixed( bool fixed )
{
  this->m_Fixed = fixed; 
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
    m_Min == levWin.GetMin() && m_Max == levWin.GetMax() &&
    m_DefaultLevel == levWin.GetDefaultLevel() && m_DefaultWindow == levWin.GetDefaultWindow() &&
    m_DefaultRangeMin == levWin.GetDefaultRangeMin() && m_DefaultRangeMax == levWin.GetDefaultRangeMax() && m_Fixed == levWin.IsFixed() ) {

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
    m_Min= levWin.GetMin();
    m_Max= levWin.GetMax();
    m_DefaultLevel = levWin.GetDefaultLevel();
    m_DefaultWindow = levWin.GetDefaultWindow();
    m_DefaultRangeMin = levWin.GetDefaultRangeMin();
    m_DefaultRangeMax = levWin.GetDefaultRangeMax();
    m_Fixed = levWin.GetFixed();
    return *this;
  }
}
