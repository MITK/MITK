/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
//! [GPUHeader]
__kernel void ckBinaryThreshold(
  __read_only image3d_t dSource, // input image
  __global uchar* dDest, // output buffer
  int lowerT, int upperT, int outsideVal, int insideVal // parameters
)
//! [GPUHeader]
