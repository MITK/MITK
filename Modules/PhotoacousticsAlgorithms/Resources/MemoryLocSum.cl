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

__kernel void ckMemoryLocSum( __global unsigned short *input, 
                         __global unsigned int *sums,
                         __global unsigned int *finalSum
                         )
 {
  uint id = get_global_id(0) + get_global_id(1) * get_global_size(0);

  uint sum = 0;
  
  for (uint pos = 0; pos < id; ++pos)
  {
    sum += input[pos];
  }
  
  sums[id] = sum;

  if (id == (get_global_size(0) - 1) + (get_global_size(1) - 1) * get_global_size(0))
    finalSum[0] = sum + input[id];
 }                                      