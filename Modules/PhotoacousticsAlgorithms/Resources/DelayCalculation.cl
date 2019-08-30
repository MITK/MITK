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

__kernel void ckDelayCalculationQuad(  __global unsigned short *gDest,
                         __global unsigned short *usedLines,
                         unsigned int inputL,
                         unsigned int inputS,
                         unsigned int outputL,
                         unsigned int outputS,
                         char isPAImage,
                         float delayMultiplicatorRaw,
                         float totalSamples_i,
                         float probeRadius,
                         float mult1,
                         float mult2 // parameters
                         )
{
  uint globalPosX = get_global_id(0);
  uint globalPosY = get_global_id(1);

  if (globalPosX * 2 < outputL && globalPosY < outputS)
  {
    float l_i = 0; // we calculate the delays relative to line zero
    float s_i = (float)globalPosY / (float)outputS * totalSamples_i;

    float l_s = (float)globalPosX / (float)outputL * (float)inputL; // the currently calculated line
    
    float elementHeight = 0;
    if (concave)
      elementHeight = (1 - cos(abs(inputL / 2.f - l_s) * mult1)) * mult2;

    float delayMultiplicator = delayMultiplicatorRaw / (s_i - elementHeight);
    
    gDest[globalPosY * (outputL / 2) + globalPosX] = delayMultiplicator * pow((l_s - l_i), 2) + (s_i - elementHeight) + (1-isPAImage)*s_i;
  }
}
 
__kernel void ckDelayCalculationSphe(  __global unsigned short *gDest,
                         __global unsigned short *usedLines,
                         unsigned int inputL,
                         unsigned int inputS,
                         unsigned int outputL,
                         unsigned int outputS,
                         char isPAImage,
                         float delayMultiplicatorRaw,
                         float totalSamples_i,
                         float probeRadius,
                         float mult1,
                         float mult2 // parameters
                         )
{
  uint globalPosX = get_global_id(0);
  uint globalPosY = get_global_id(1);

  if (globalPosX * 2 < outputL && globalPosY < outputS)
  {
    float l_i = 0; // we calculate the delays relative to line zero
    float s_i = (float)globalPosY / (float)outputS * totalSamples_i;
    float l_s = (float)globalPosX / (float)outputL * (float)inputL; // the currently calculated line

    float elementHeight = 0;
    if (concave)
      elementHeight = (1 - cos(abs(inputL / 2.f - l_s) * mult1)) * mult2;
      
    gDest[globalPosY * (outputL / 2) + globalPosX] =
      sqrt(
        pow(s_i - elementHeight, 2)
        +
        pow((delayMultiplicatorRaw * ((l_s - l_i)) / inputL), 2)
      ) + (1-isPAImage)*s_i;
  }
}