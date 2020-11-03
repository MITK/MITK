/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

__kernel void ckDAS_g(
  __global float* dSource, // input image
  __global float* dDest, // output buffer
  __global float* elementHeights,
  __global float* elementPositions,
  __constant float* apodArray,
  unsigned short apodArraySize,
  unsigned int inputL,
  unsigned int inputS,
  int Slices,
  int outputL,
  int outputS,
  float totalSamples_i,
  float horizontalExtent,
  float mult,
  char isPAImage,
  __global unsigned short* usedLines // parameters
)
{
  // get thread identifier
  int globalPosX = get_global_id(0);
  int globalPosY = get_global_id(1);
  int globalPosZ = get_global_id(2);

  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS && globalPosZ < Slices )
  {
    int AddSample = 0;
    float l_p = 0;
    float s_i = 0;

    float apod_mult = 1;

    float output = 0;

    l_p = (float)globalPosX / outputL * horizontalExtent;
    s_i = (float)globalPosY / outputS * totalSamples_i;

    unsigned short curUsedLines = usedLines[globalPosY * 3 * outputL + 3 * globalPosX];
    unsigned short minLine = usedLines[globalPosY * 3 * outputL + 3 * globalPosX + 1];
    unsigned short maxLine = usedLines[globalPosY * 3 *outputL + 3 * globalPosX + 2];

    apod_mult = (float)apodArraySize / curUsedLines;

    for (int l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = (int)sqrt(
        pow(s_i-elementHeights[l_s]*mult, 2)
        +
        pow(mult * (l_p - elementPositions[l_s]), 2)
      ) + (1 - isPAImage)*s_i;
      if (AddSample < inputS && AddSample >= 0)
        output += dSource[(int)(globalPosZ * inputL * inputS + l_s + AddSample*inputL)] * apodArray[(int)((l_s - minLine)*apod_mult)];
      else
        --curUsedLines;
    }
    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / curUsedLines;
  }
}
