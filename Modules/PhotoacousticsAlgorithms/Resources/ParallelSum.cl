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

__kernel void ckParallelSum( __const const unsigned short *input, 
                         __global unsigned int *partialSums,
                         __local unsigned int *localSums)
 {
  uint local_id = get_local_id(0) + get_local_id(1) * get_local_size(2);
  uint group_size = get_local_size(0) * get_local_size(1);

  localSums[local_id] = input[get_global_id(0) + get_global_id(1) * get_global_size(0)];

  for (uint stride = group_size/2; stride>0; stride /=2)
     {
      barrier(CLK_LOCAL_MEM_FENCE);

      if (local_id < stride)
        localSums[local_id] += localSums[local_id + stride];
     }

  if (local_id == 0)
    partialSums[get_group_id(0)] = localSums[0];
 }                                      