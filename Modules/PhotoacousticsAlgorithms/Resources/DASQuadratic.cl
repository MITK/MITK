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
  __read_only image3d_t dSource, // input image
  __global float* dDest, // output buffer
  __global float* apodArray,
  unsigned short apodArraySize,
  float SpeedOfSound,
  float TimeSpacing,
  float Pitch,
  float Angle,
  unsigned short PAImage,
  unsigned short TransducerElements  // parameters
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);
  unsigned int globalPosZ = get_global_id(2);
  
  unsigned short outputS = get_global_size(1);
  unsigned short outputL = get_global_size(0);
  
  // get image width and weight
  const unsigned int inputL = get_image_width( dSource );
  const unsigned int inputS = get_image_height( dSource );
  const unsigned int Slices = get_image_depth( dSource );

  // create an image sampler
  const sampler_t defaultSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST ;

  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS && globalPosZ < Slices )
  {	
    float l_i = (float)globalPosX / outputL * inputL;
    float s_i = (float)globalPosY / outputS * inputS / 2;
	
    float part = (tan(Angle / 360 * 2 * M_PI) * TimeSpacing * SpeedOfSound / Pitch * outputL / TransducerElements) * s_i;
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
      if (AddSample < inputS && AddSample >= 0) 
        output += apodArray[(short)((l_s - minLine)*apod_mult)] * read_imagef( dSource, defaultSampler, (int4)(l_s, AddSample, globalPosZ, 0 )).x;
      else
        --usedLines;
    }
	
    dDest[ globalPosZ * outputL * outputS + globalPosY * outputL + globalPosX ] = output / usedLines;
  }
}