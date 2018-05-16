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

__kernel void ckBmodeAbs(
  __global float* dSource, // input image
  __global float* dDest, // output buffer
  unsigned int size
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);
  unsigned int globalPosZ = get_global_id(2);
  
  // get image width and height
  unsigned short inputS = get_global_size(1);
  unsigned short inputL = get_global_size(0);
  unsigned short slices = get_global_size(2);

  // terminate non-valid threads
  if ( globalPosX + inputL * globalPosY + inputL * inputS * globalPosZ < size )
  {
    dDest[ globalPosZ * inputL * inputS + globalPosY * inputL + globalPosX ] = fabs(dSource[ globalPosZ * inputL * inputS + globalPosY * inputL + globalPosX ]);
  }
}