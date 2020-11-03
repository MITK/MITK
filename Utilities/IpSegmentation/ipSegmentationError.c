/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <stdio.h>
#include "ipSegmentationP.h"

static const char* messages [] = {
  "ok.",
  "sorry, out of memory.",
  "sorry, wrong data type of segmentation image.",
  "sorry, undo is disabled.",
  "sorry, PIC descriptor is null.",
  "sorry, unknown error occurred."
};

void
ipMITKSegmentationError (int error)
{
  switch (error) {
  case ipMITKSegmentationOK:
  case ipMITKSegmentationOUT_OF_MEMORY:
  case ipMITKSegmentationWRONG_TYPE:
  case ipMITKSegmentationUNDO_DISABLED:
    printf ("ipMITKSegmentation: %s\n", messages [error]);
    break;
  default:
    printf ("ipMITKSegmentation: %s\n", messages [ipMITKSegmentationUNKNOWN_ERROR]);
    break;
  }
  if (error > 0) {
    exit (-1);
  }
}

void
ipMITKSegmentationCheckImage (mitkIpPicDescriptor* segmentation)
{
  if ((segmentation->type != ipMITKSegmentationTYPE_ID) || (segmentation->bpe != ipMITKSegmentationBPE)) {
    ipMITKSegmentationError (ipMITKSegmentationWRONG_TYPE);
  }
  if (!segmentation->data) {
    ipMITKSegmentationError (ipMITKSegmentationUNKNOWN_ERROR);
  }
}
