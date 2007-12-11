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

#include<mitkColorConversions.h>

namespace mitk {

namespace ColorConversions {

/*
   Convert HSV color to RGB color (pseudocode from wikipedia)
   
   H from 0 to 360
   S from 0 to 1
   V from 0 to 1

   R, G, and B from 0 to 1
*/
void Hsv2Rgb(float h, float s, float v, float& r, float& g, float& b)
{
  if (s == 0.0)
  {
    r = g = b = v; // gray
    return;
  }

  int Hi = (int)( h / 60.0 ) % 6;
  if (h >= 360 ) Hi = 6;

  float f = (float)(h / 60.0 - (float)Hi);
  float p = (float)(v * (1.0 - s));
  float q = (float)(v * (1.0 - s * f));
  float t = (float)(v * (1.0 - s * (1.0 - f)));

  switch (Hi)
  {
    case 0: case 6: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
}
/*
   Convert RGB color to HSV color 
   
   R from 0 to 1
   G from 0 to 1
   B from 0 to 1

   H from 0 to 360 and -1 for undefined color (pure white)
   S from 0 to 1
   V from 0 to 1
*/
void Rgb2Hsv(float r, float g, float b, float &h, float &s, float &v) 
    { 
       /* r = r/255;
        b = b/255;
        g = g/255;*/
        
        float mn=r,mx=r; 
        int maxVal=0; 
      
        if (g > mx){ mx=g;maxVal=1;} 
        if (b > mx){ mx=b;maxVal=2;}  
        if (g < mn) mn=g; 
        if (b < mn) mn=b;  

        float  delta = mx - mn; 
      
        v = mx;  
        if( mx != 0 ) 
          s = delta / mx;  
        else  
        { 
          s = 0; 
          h = 0; 
          return; 
        } 
        if (s==0.0f) 
        { 
          h=-1; 
          return; 
        } 
        else 
        {  
          switch (maxVal) 
          { 
          case 0:{h = ( g - b ) / delta;break;}         // yel < h < mag 
          case 1:{h = 2 + ( b - r ) / delta;break;}     // cyan < h < yel 
          case 2:{h = 4 + ( r - g ) / delta;break;}     // mag < h < cyan 
          } 
        } 
      
        h *= 60; 
        if( h < 0 ) h += 360; 
    }

} // ColorConversion
  
} // mitk

