/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

__kernel void ckDMAS(
  __global float* dSource, // input image
  __global float* dDest, // output buffer
  __global unsigned short* usedLines,
  __global unsigned short* AddSamples,
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

    unsigned short Delay1 = 0;
    unsigned short Delay2 = 0;

    float output = 0;
    float mult = 0;

    float s_1 = 0;
    float s_2 = 0;
    float apod_1 = 0;

    for (short l_s1 = minLine; l_s1 < maxLine; ++l_s1)
    {
      Delay1 = AddSamples[globalPosY * (outputL / 2) + (int)(fabs(l_s1 - l_i)/(float)inputL * (float)outputL)];
      if (Delay1 < inputS && Delay1 >= 0)
      {
        s_1 = dSource[(int)(globalPosZ * inputL * inputS + Delay1 * inputL + l_s1)];
        apod_1 = apodArray[(int)((l_s1 - minLine)*apod_mult)];

        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          Delay2 = AddSamples[globalPosY * (outputL / 2) + (int)(fabs(l_s2 - l_i)/(float)inputL * (float)outputL)];
          if (Delay2 < inputS && Delay2 >= 0)
          {
            s_2 = dSource[(int)(globalPosZ * inputL * inputS + Delay2 * inputL + l_s2)];

            mult = apodArray[(int)((l_s2 - minLine)*apod_mult)] * s_2 * apod_1 * s_1;

            output += sqrt(fabs(mult)) * sign(mult);
          }
        }
      }
      else
        --curUsedLines;
    }

    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / (float)(curUsedLines * curUsedLines - (curUsedLines - 1));
  }
}

__kernel void ckDMAS_g(
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
    int AddSample1 = 0;
    int AddSample2 = 0;

    float output = 0;

    float s_1 = 0;
    float s_2 = 0;
    float apod_1 = 0;

    float l_p = (float)globalPosX / outputL * horizontalExtent;
    float s_i = (float)globalPosY / outputS * totalSamples_i;

    unsigned short curUsedLines = usedLines[globalPosY * 3 * outputL + 3 * globalPosX];
    unsigned short minLine = usedLines[globalPosY * 3 * outputL + 3 * globalPosX + 1];
    unsigned short maxLine = usedLines[globalPosY * 3 *outputL + 3 * globalPosX + 2];

    float apod_mult = (float)apodArraySize / curUsedLines;

    float multiplication = 0;

    for (int l_s1 = minLine; l_s1 < maxLine; ++l_s1)
    {
      AddSample1 = (int)sqrt(
        pow(s_i-elementHeights[l_s1]*mult, 2)
        +
        pow(mult * (l_p - elementPositions[l_s1]), 2)
      ) + (1 - isPAImage)*s_i;

      if (AddSample1 < inputS && AddSample1 >= 0)
      {
        s_1 = dSource[(int)(globalPosZ * inputL * inputS + AddSample1 * inputL + l_s1)];
        apod_1 = apodArray[(int)((l_s1 - minLine)*apod_mult)];

        for (int l_s2 = minLine; l_s2 < maxLine; ++l_s2)
        {
          AddSample2 = (int)sqrt(
            pow(s_i-elementHeights[l_s2]*mult, 2)
            +
            pow(mult * (l_p - elementPositions[l_s2]), 2)
          ) + (1 - isPAImage)*s_i;
          if (AddSample2 < inputS && AddSample2 >= 0)
          {
            s_2 = dSource[(int)(globalPosZ * inputL * inputS + AddSample2 * inputL + l_s2)];
            multiplication = apodArray[(int)((l_s2 - minLine)*apod_mult)] * s_2 * apod_1 * s_1;

            output += sqrt(fabs(multiplication)) * sign(multiplication);
          }
        }
      }
      else
        --curUsedLines;
    }
    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / (float)(pow((float)curUsedLines, 2) - (curUsedLines - 1));
  }
}
