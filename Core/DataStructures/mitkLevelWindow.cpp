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
  testValues();
}

void mitk::LevelWindow::SetRangeMax(mitk::ScalarType max)
{
  m_RangeMax = max;
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
  return  (m_RangeMax > 0) ? (m_RangeMax - m_RangeMin) : (m_RangeMin - m_RangeMax);
}

void mitk::LevelWindow::SetAuto(mitk::Image* image, bool tryPicTags)
{
  if( image == NULL )
    return;

  if(tryPicTags)
  {
    if(SetAutoByPicTags(image->GetPic()))
      return;
  }
  SetMinMax(image->GetScalarValue2ndMin(), image->GetScalarValue2ndMax());
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
