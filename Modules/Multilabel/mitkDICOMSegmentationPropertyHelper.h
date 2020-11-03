/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef DICOMPMPROPERTYHELPER_H_
#define DICOMPMPROPERTYHELPER_H_

#include <mitkDICOMTag.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>

#include <MitkMultilabelExports.h>

namespace mitk
{
  class MITKMULTILABEL_EXPORT DICOMSegmentationPropertyHelper
  {
  public:
    static void DeriveDICOMSegmentationProperties(LabelSetImage* dicomSegImage);
    static void SetDICOMSegmentProperties(Label *label);
  };
}
#endif
