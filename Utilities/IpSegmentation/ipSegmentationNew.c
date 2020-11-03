/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "ipSegmentationP.h"

mitkIpPicDescriptor*
ipMITKSegmentationNew (mitkIpPicDescriptor* image)
{
    mitkIpPicDescriptor* s = NULL;

    if (image) {
        s = mitkIpPicNew ();
        s->type = ipMITKSegmentationTYPE_ID;
        s->bpe = ipMITKSegmentationBPE;
        s->dim = 2;
        s->n[0] = image->n[0];
        s->n[1] = image->n[1];
        s->data = malloc (_mitkIpPicSize (s));
    }
    return s;
}
