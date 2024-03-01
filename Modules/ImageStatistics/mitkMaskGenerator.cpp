/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMaskGenerator.h>

namespace mitk
{

MaskGenerator::MaskGenerator():
    m_TimePoint(0)
{
}

mitk::Image::ConstPointer MaskGenerator::GetMask(unsigned int maskID)
{
  if (maskID >= this->GetNumberOfMasks()) mitkThrow() << "Cannot generate and return mask. Passed mask ID is invalid. Invalid ID: " << maskID;
        this->Modified();

  return this->DoGetMask(maskID);
}

mitk::Image::ConstPointer MaskGenerator::GetReferenceImage()
{
    return m_InputImage;
}
}
