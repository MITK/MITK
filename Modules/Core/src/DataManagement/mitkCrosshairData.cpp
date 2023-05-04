/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrosshairData.h>

mitk::CrosshairData::CrosshairData()
{
}

mitk::CrosshairData::~CrosshairData()
{
}

void mitk::CrosshairData::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::CrosshairData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::CrosshairData::VerifyRequestedRegion()
{
  return true;
}

void mitk::CrosshairData::SetRequestedRegion(const itk::DataObject*)
{
}
