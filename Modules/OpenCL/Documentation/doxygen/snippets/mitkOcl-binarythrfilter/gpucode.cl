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
//! [GPUHeader]
__kernel void ckBinaryThreshold(
  __read_only image3d_t dSource, // input image
  __global uchar* dDest, // output buffer
  int lowerT, int upperT, int outsideVal, int insideVal // parameters
)
//! [GPUHeader]
