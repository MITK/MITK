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

__kernel void ckDMAS(
  __global float* dSource, // input image
  __global float* dDest, // output buffer
  __global unsigned short* usedLines,
  __global unsigned int* memoryLocations,
  __global unsigned short* AddSamples,
  __global float* apodArray,
  unsigned short apodArraySize,
  unsigned int inputL,
  unsigned int inputS,
  unsigned int Slices,
  unsigned int outputL,
  unsigned int outputS  // parameters
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);
  unsigned int globalPosZ = get_global_id(2);

  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS && globalPosZ < Slices )
  {	
    unsigned short curUsedLines = usedLines[globalPosY * 3 * outputL + 3 * globalPosX];
    unsigned short minLine = usedLines[globalPosY * 3 * outputL + 3 * globalPosX + 1];
    unsigned short maxLine = usedLines[globalPosY * 3 *outputL + 3 * globalPosX + 2];
    
    float apod_mult = (float)apodArraySize / (float)curUsedLines;
    
    unsigned short AddSample1 = 0;
    unsigned short AddSample2 = 0;
    
    float output = 0;
    float mult = 0;
    
    unsigned int MemoryStartAccessPoint = memoryLocations[globalPosY * outputL + globalPosX];

    for (short l_s1 = minLine; l_s1 < maxLine; ++l_s1)
    {
      AddSample1 = AddSamples[MemoryStartAccessPoint + l_s1 - minLine];
      if (AddSample1 < inputS && AddSample1 >= 0) {
        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          AddSample2 = AddSamples[MemoryStartAccessPoint + l_s2 - minLine];
          if (AddSample1 < inputS && AddSample1 >= 0) {
            mult = apodArray[(int)((l_s2 - minLine)*apod_mult)] * 
              dSource[(int)(globalPosZ * inputL * inputS + AddSample2 * inputL + l_s2)]
              * apodArray[(int)((l_s1 - minLine)*apod_mult)] * 
              dSource[(int)(globalPosZ * inputL * inputS + AddSample1 * inputL + l_s1)];
              
            output += sqrt(mult * ((float)(mult>0)-(float)(mult<0))) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --curUsedLines;
    }
    
    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / (pow((float)curUsedLines, 2.0f) - (curUsedLines - 1));
  }
}