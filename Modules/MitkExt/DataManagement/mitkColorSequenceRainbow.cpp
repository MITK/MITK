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
}//end of constructor


ColorSequenceRainbow::~ColorSequenceRainbow()
{

}//end of destructor

Color ColorSequenceRainbow::GetNextColor()
{
  m_Color = m_ColorList[m_ColorIndex];
  if(m_ColorIndex < m_ColorList.size()-1)
  {
    m_ColorIndex++;
  }
  else
  {
    m_ColorIndex = 0;
  }
  return m_Color;
}//end of GetNextMitkColor()

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

  defaultColor.Set( (float) 90/255, (float) 25/255, (float) 72/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 69/255, (float) 42/255, (float) 47/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 141/255, (float) 188/255, (float) 56/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 85/255, (float) 198/255, (float) 76/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 42/255, (float) 150/255, (float) 99/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 142/255, (float) 150/255, (float) 29/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 14/255, (float) 15/255, (float) 129/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 54/255, (float) 215/255, (float) 89/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 154/255, (float) 115/255, (float) 89/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 15/255, (float) 95/255, (float) 189/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 215/255, (float) 195/255, (float) 19/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 55/255, (float) 95/255, (float) 219/255 );
  m_ColorList.push_back(defaultColor);
  defaultColor.Set( (float) 155/255, (float) 220/255, (float) 129/255 );
  m_ColorList.push_back(defaultColor);

  m_ColorList.push_back(red);
  m_ColorList.push_back(yellow);
  m_ColorList.push_back(blue);
  m_ColorList.push_back(green);
  m_ColorList.push_back(magenta);
  m_ColorList.push_back(cyan);
  m_ColorList.push_back(orange);
  m_ColorList.push_back(defaultColor);

}// end of ColorList

void ColorSequenceRainbow::GoToBegin()
{
  m_ColorIndex = 0;
}

}//end of namespace

