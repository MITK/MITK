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
                         __global unsigned int *memoryLocations,
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
    uint globalPosZ = get_global_id(2);
    
    if(globalPosZ < usedLines[globalPosY * 3 * outputL + 3 * globalPosX])
    {
      float l_i = (float)globalPosX / outputL * inputL;
      float s_i = (float)globalPosY / outputS * inputS / 2;
      
      float l_s = usedLines[globalPosY * 3 * outputL + 3 * globalPosX + 1] + globalPosZ;

      float delayMultiplicator = delayMultiplicatorRaw / s_i;
      float AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i + (1-isPAImage)*s_i;
      gDest[memoryLocations[globalPosY * outputL + globalPosX] + globalPosZ] = AddSample;
    }
 }
 
 __kernel void ckDelayCalculationSphe(  __global unsigned short *gDest,
                         __global unsigned short *usedLines, 
                         __global unsigned int *memoryLocations,
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
    uint globalPosZ = get_global_id(2);
    
    if(globalPosZ < usedLines[globalPosY * 3 * outputL + 3 * globalPosX])
    {
      float l_i = (float)globalPosX / outputL * inputL;
      float s_i = (float)globalPosY / outputS * inputS / 2;
      
      float l_s = usedLines[globalPosY * 3 * outputL + 3 * globalPosX + 1] + globalPosZ;

      gDest[memoryLocations[globalPosY * outputL + globalPosX] + globalPosZ] = 
        sqrt(
          pow(s_i, 2)
          +
          pow((delayMultiplicatorRaw * ((l_s - l_i)) / inputL), 2)
        ) + (1-isPAImage)*s_i;
    }
 }