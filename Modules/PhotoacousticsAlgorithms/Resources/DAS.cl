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

__kernel void ckDAS(
  __global float* dSource, // input image
  __global float* dDest, // output buffer
  __global unsigned short* usedLines,
  __global unsigned short* delays,
  __constant float* apodArray,
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
    float l_i = (float)globalPosX / (float)outputL * (float)inputL;

    unsigned short curUsedLines = usedLines[globalPosY * 3 * outputL + 3 * globalPosX];
    unsigned short minLine = usedLines[globalPosY * 3 * outputL + 3 * globalPosX + 1];
    unsigned short maxLine = usedLines[globalPosY * 3 *outputL + 3 * globalPosX + 2];
    
    float apod_mult = (float)apodArraySize / (float)curUsedLines;
    
    unsigned short Delay = 0;
    
    float output = 0;
    float mult = 0;
    
    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      Delay = delays[globalPosY * (outputL / 2) + (int)(fabs(l_s - l_i)/(float)inputL * (float)outputL)];
      if (Delay < inputS && Delay >= 0) {
        output += apodArray[(int)((l_s - minLine)*apod_mult)] * dSource[(int)(globalPosZ * inputL * inputS + Delay * inputL + l_s)];
      }
      else
        --curUsedLines;
    }
    
    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / (float)curUsedLines;
  }
}