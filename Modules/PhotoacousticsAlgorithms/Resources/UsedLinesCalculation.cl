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

__kernel void ckUsedLines(
  __global unsigned short* dDest, // output buffer
  float partMult,
  unsigned int inputL,
  unsigned int inputS,
  unsigned int outputL,
  unsigned int outputS,
  float totalSamples_i // parameters
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);
  
  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS)
  {
    float l_i = (float)globalPosX / outputL * inputL;
    float s_i = (float)globalPosY / (float)outputS * totalSamples_i;

    float part = partMult * s_i;
    if (part < 1)
      part = 1;
    
    unsigned short maxLine = min((l_i + part) + 1, (float)inputL);
    unsigned short minLine = max((l_i - part), 0.0f);
    
    dDest[globalPosY * 3 * outputL + 3 * globalPosX] = (maxLine - minLine); //usedLines
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 1] = minLine; //minLine
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 2] = maxLine; //maxLine
  }
}

__kernel void ckUsedLines_g(
  __global unsigned short* dDest, // output buffer
  __global float* elementHeights,
  __global float* elementPositions,
  float cos_deg,
  float probeRadius,
  unsigned int inputL,
  unsigned int inputS,
  unsigned int outputL,
  unsigned int outputS,
  float horizontalExtent,
  float verticalExtent
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);
  
  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS)
  {
    float cos = 0;
    float a = 0;
    float d = 0;
    
    float l_p = (float)globalPosX / outputL * horizontalExtent;
    float s_p = (float)globalPosY / (float)outputS * verticalExtent;
    
    int maxLine = inputL;
    int minLine = 0;
    
    for(int l_s = 0; l_s < inputL; l_s+=32)
    {
      a = sqrt((probeRadius - s_p)*(probeRadius - s_p) + (l_p - horizontalExtent/2)*(l_p - horizontalExtent/2));
      d = sqrt((s_p - elementHeights[l_s])*(s_p - elementHeights[l_s]) + (l_p - elementPositions[l_s])*(l_p - elementPositions[l_s]));
      cos = (d*d + probeRadius*probeRadius - a*a)/(2*probeRadius*d);
      
      if(cos > cos_deg)
      {
        minLine = l_s-32;
        if(minLine < 0)
          minLine = 0;
        break;
      }
    }
    for(int l_s = minLine; l_s < inputL; l_s+=8)
    {
      a = sqrt((probeRadius - s_p)*(probeRadius - s_p) + (l_p - horizontalExtent/2)*(l_p - horizontalExtent/2));
      d = sqrt((s_p - elementHeights[l_s])*(s_p - elementHeights[l_s]) + (l_p - elementPositions[l_s])*(l_p - elementPositions[l_s]));
      cos = (d*d + probeRadius*probeRadius - a*a)/(2*probeRadius*d);
      
      if(cos > cos_deg)
      {
        minLine = l_s-8;
        if(minLine < 0)
          minLine = 0;
        break;
      }
    }
    for(int l_s = minLine; l_s < inputL; l_s+=1)
    {
      a = sqrt((probeRadius - s_p)*(probeRadius - s_p) + (l_p - horizontalExtent/2)*(l_p - horizontalExtent/2));
      d = sqrt((s_p - elementHeights[l_s])*(s_p - elementHeights[l_s]) + (l_p - elementPositions[l_s])*(l_p - elementPositions[l_s]));
      cos = (d*d + probeRadius*probeRadius - a*a)/(2*probeRadius*d);
      
      if(cos > cos_deg)
      {
        minLine = l_s;
        break;
      }
    }
    
    for(int l_s = inputL; l_s >= 0 ; l_s-=32)
    {      
      a = sqrt((probeRadius - s_p)*(probeRadius - s_p) + (l_p - horizontalExtent/2)*(l_p - horizontalExtent/2));
      d = sqrt((s_p - elementHeights[l_s])*(s_p - elementHeights[l_s]) + (l_p - elementPositions[l_s])*(l_p - elementPositions[l_s]));
      cos = (d*d + probeRadius*probeRadius - a*a)/(2*probeRadius*d);
      cos = 0;
      
      if(cos > cos_deg)
      {
        maxLine = l_s+32;
        if(maxLine > inputL)
          minLine = inputL;
        break;
      }
    }
    for(int l_s = maxLine; l_s >= 0 ; l_s-=8)
    {      
      a = sqrt((probeRadius - s_p)*(probeRadius - s_p) + (l_p - horizontalExtent/2)*(l_p - horizontalExtent/2));
      d = sqrt((s_p - elementHeights[l_s])*(s_p - elementHeights[l_s]) + (l_p - elementPositions[l_s])*(l_p - elementPositions[l_s]));
      cos = (d*d + probeRadius*probeRadius - a*a)/(2*probeRadius*d);
      cos = 0;
      
      if(cos > cos_deg)
      {
        maxLine = l_s+8;
        if(maxLine > inputL)
          minLine = inputL;
        break;
      }
    }
    for(int l_s = maxLine; l_s >= 0 ; l_s-=1)
    {      
      a = sqrt((probeRadius - s_p)*(probeRadius - s_p) + (l_p - horizontalExtent/2)*(l_p - horizontalExtent/2));
      d = sqrt((s_p - elementHeights[l_s])*(s_p - elementHeights[l_s]) + (l_p - elementPositions[l_s])*(l_p - elementPositions[l_s]));
      cos = (d*d + probeRadius*probeRadius - a*a)/(2*probeRadius*d);
      cos = 0;
      
      if(cos > cos_deg)
      {
        maxLine = l_s;
        break;
      }
    }
    
    dDest[globalPosY * 3 * outputL + 3 * globalPosX] = (maxLine - minLine); //usedLines
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 1] = minLine; //minLine
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 2] = maxLine; //maxLine
  }
}