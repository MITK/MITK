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

__kernel void ckBinaryThreshold(
  __read_only image3d_t dSource, // input image
  __global uchar* dDest, // output buffer
  int lowerT, int upperT, int outsideVal, int insideVal // parameters
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);
  unsigned int globalPosZ = get_global_id(2);

  // get image width and weight
  const unsigned int uiWidth = get_image_width( dSource );
  const unsigned int uiHeight = get_image_height( dSource );
  const unsigned int uiDepth = get_image_depth( dSource );

  // create an image sampler
  const sampler_t defaultSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST ;

  // terminate non-valid threads
  if ( globalPosX < uiWidth && globalPosY < uiHeight && globalPosZ < uiDepth )
  {
    int result = outsideVal;

    // get input value
    int4 pixelValue = read_imagei( dSource, defaultSampler, (int4) (globalPosX, globalPosY, globalPosZ, 0 ));

    if( (pixelValue.x >= lowerT) && (pixelValue.x <= upperT ) )
    {
      result = insideVal;
    }

    // store the result
    dDest[ globalPosZ * uiWidth * uiHeight + globalPosY * uiWidth + globalPosX ] = result;
  }

}
