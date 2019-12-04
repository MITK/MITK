/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkColorSequenceRainbow.h"
#include <vector>
namespace mitk
{
  ColorSequenceRainbow::ColorSequenceRainbow()
  {
    m_ColorIndex = 0;
    m_Color.SetRed(1.0);
    m_Color.SetGreen(0.67);
    m_Color.SetBlue(0.0);
    InitColorList();
  } // end of constructor

  ColorSequenceRainbow::~ColorSequenceRainbow() {} // end of destructor
  Color ColorSequenceRainbow::GetNextColor()
  {
    if (m_ColorIndex < (m_ColorList.size() - 1))
    {
      m_ColorIndex++;
    }
    else
    {
      m_ColorIndex = 0;
    }
    m_Color = m_ColorList[m_ColorIndex];
    return m_Color;
  } // end of GetNextMitkColor()

  void ColorSequenceRainbow::InitColorList()
  {
    mitk::Color defaultColor, red, green, blue, yellow, magenta, cyan, orange;
    defaultColor.Set(1.0, 0.67, 0.0);
    red.Set(1.0, 0.0, 0.0);
    green.Set(0.0, 1.0, 0.0);
    blue.Set(0.0, 0.0, 1.0);
    yellow.Set(1.0, 1.0, 0.0);
    magenta.Set(1.0, 0.0, 1.0);
    cyan.Set(0.0, 1.0, 1.0);
    orange.Set(1.0, 0.6, 0.0);
    m_ColorList.push_back(defaultColor);
    m_ColorList.push_back(red);
    m_ColorList.push_back(yellow);
    m_ColorList.push_back(blue);
    m_ColorList.push_back(green);
    m_ColorList.push_back(magenta);
    m_ColorList.push_back(cyan);
    m_ColorList.push_back(orange);
  } // end of ColorList

  void ColorSequenceRainbow::GoToBegin() { m_ColorIndex = 0; }
} // end of namespace
