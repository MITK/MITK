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

#include "mitkAutoSegmentationTool.h"
#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include "mitkSliceNavigationController.h"
#include "mitkRenderingManager.h"

mitk::AutoSegmentationTool::AutoSegmentationTool() : SegTool3D("dummy")
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

const char* mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
}
