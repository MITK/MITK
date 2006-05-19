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
  float f = (float)(h / 60.0 - Hi);
  float p = (float)(v * (1 - s));
  float q = (float)(v * (1 - s * f));
  float t = (float)(v * (1 - s * (1 - f)));

  switch (Hi)
  {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
}

} // ColorConversion
  
} // mitk

