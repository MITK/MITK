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
