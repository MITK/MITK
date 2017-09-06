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

__kernel void ckDASQuad(
  __global float* dSource, // input image
  __global float* dDest, // output buffer
  __global float* apodArray,
  unsigned short apodArraySize,
  float SpeedOfSound,
  float TimeSpacing,
  float Pitch,
  float Angle,
  unsigned short PAImage,
  unsigned short TransducerElements,
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
    float l_i = (float)globalPosX / outputL * inputL;
    float s_i = (float)globalPosY / outputS * inputS / 2;
	
    float part = (tan(Angle / 360 * 2 * M_PI) * ((SpeedOfSound * TimeSpacing) * s_i) / (Pitch * TransducerElements)) * inputL;
    if (part < 1)
      part = 1;

    short maxLine = min((l_i + part) + 1, (float)inputL);
    short minLine = max((l_i - part), 0.0f);
    short usedLines = (maxLine - minLine);
    float apod_mult = apodArraySize / (maxLine - minLine);
    
    short AddSample = 0;
    
    float output = 0;

    float delayMultiplicator = pow(1 / (TimeSpacing*SpeedOfSound) * Pitch * TransducerElements / inputL, 2) / s_i / 2;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i + (1-PAImage)*s_i;
      if (AddSample < inputS && AddSample >= 0) {
        output += apodArray[(short)((l_s - minLine)*apod_mult)] * dSource[(unsigned int)(globalPosZ * inputL * inputS + AddSample * inputL + l_s)];
        }
      else
        --usedLines;
    }
	  
    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / usedLines;
  }
}