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
                         float delayMultiplicatorRaw // parameters
                         )
{
  uint globalPosX = get_global_id(0);
  uint globalPosY = get_global_id(1);

  if (globalPosX * 2 < outputL && globalPosY < outputS)
  {
    float l_i = 0; // we calculate the delays relative to line zero
    float s_i = (float)globalPosY / (float)outputS * (float)inputS / 2;

    float l_s = (float)globalPosX / (float)outputL * (float)inputL; // the currently calculated line

    float delayMultiplicator = delayMultiplicatorRaw / s_i;
    gDest[globalPosY * (outputL / 2) + globalPosX] = delayMultiplicator * pow((l_s - l_i), 2) + s_i + (1-isPAImage)*s_i;
  }
}
 
__kernel void ckDelayCalculationSphe(  __global unsigned short *gDest,
                         __global unsigned short *usedLines,
                         unsigned int inputL,
                         unsigned int inputS,
                         unsigned int outputL,
                         unsigned int outputS,
                         char isPAImage,
                         float delayMultiplicatorRaw // parameters
                         )
{
  uint globalPosX = get_global_id(0);
  uint globalPosY = get_global_id(1);

  if (globalPosX * 2 < outputL && globalPosY < outputS)
  {
    float l_i = 0; // we calculate the delays relative to line zero
    float s_i = (float)globalPosY / (float)outputS * (float)inputS / 2;

    float l_s = (float)globalPosX / (float)outputL * (float)inputL; // the currently calculated line

    gDest[globalPosY * (outputL / 2) + globalPosX] =
      sqrt(
        pow(s_i, 2)
        +
        pow((delayMultiplicatorRaw * ((l_s - l_i)) / inputL), 2)
      ) + (1-isPAImage)*s_i;
  }
}