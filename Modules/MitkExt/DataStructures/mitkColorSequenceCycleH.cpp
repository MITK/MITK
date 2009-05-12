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
#include <mitkColorSequenceCycleH.h>
#include <mitkColorConversions.h>

namespace mitk
{

ColorSequenceCycleH::ColorSequenceCycleH()
{
  color_h = -60.0; // after the first increase this will yield red
  color_s = 1.0;  // full saturation
  color_v = 1.0; // full value
  color_cycle = 0;
}

ColorSequenceCycleH::~ColorSequenceCycleH()
{
  // nothing to do
}

Color ColorSequenceCycleH::GetNextColor()
{
  color_h += 60.0;
  if ( color_h < 0.0 ) color_h = 0.0; 

  if (color_h >= 360.0)
  {
    if ( color_cycle == 0 )
    {
      color_h = 30.0;
      color_s = 1.0;
      color_v = 1.0;
      color_cycle = 1;
    }
    else if (color_cycle == 1)
    {
      color_h = 0.0;
      color_s = 0.5;
      color_v = 1.0;
      color_cycle = 2;
    }
    else if (color_cycle == 2)
    {
      color_h = 30.0;
      color_s = 0.5;
      color_v = 1.0;
      color_cycle = 3;
    }
    else if (color_cycle == 3)
    {
      color_h = 0.0;
      color_s = 1.0;
      color_v = 0.5;
      color_cycle = 4;
    }
    else if (color_cycle == 4)
    {
      color_h = 30.0;
      color_s = 1.0;
      color_v = 0.5;
      color_cycle = 5;
    }
    else if (color_cycle == 5)
    {
      color_h = 0.0;
      color_s = 1.0;
      color_v = 1.0;
      color_cycle = 0;
    }
  }

  // convert to rgb
  float r, g, b;
  ColorConversions::Hsv2Rgb(color_h, color_s, color_v, r, g, b);
  
  Color returnColor;
  returnColor.Set(r, g, b);
  
  return returnColor;
}

void ColorSequenceCycleH::GoToBegin()
{
  color_h = -60.0; // after the first increase this will yield red
  color_s = 1.0;  // full saturation
  color_v = 1.0; // full value
  color_cycle = 0;
}

void ColorSequenceCycleH::ChangeHueValueByCycleSteps( int steps )
{
  color_h += (float)(steps*60.0);
}

void ColorSequenceCycleH::ChangeHueValueByAbsoluteNumber( float number )
{
  color_h += number;
}

void ColorSequenceCycleH::SetColorCycle( unsigned short cycle )
{
if ( cycle > 5 ) cycle = 5;
color_cycle = cycle;
color_h = 360.0;
this->GetNextColor();

}

}  // mitk

