/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROIIO.h"
#include <mitkROI.h>
#include <mitkROIIOMimeTypes.h>

mitk::ROIIO::ROIIO()
  : AbstractFileIO(ROI::GetStaticNameOfClass(), MitkROIIOMimeTypes::ROI_MIMETYPE(), "MITK ROI")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::ROIIO::DoRead()
{
  std::vector<BaseData::Pointer> result;
  return result;
}

void mitk::ROIIO::Write()
{
}

mitk::ROIIO* mitk::ROIIO::IOClone() const
{
  return new ROIIO(*this);
}
