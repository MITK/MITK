/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVirtualTrackingTool.h"
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::VirtualTrackingTool::VirtualTrackingTool()
: TrackingTool(), m_Spline(SplineType::New()), m_SplineLength(0.0), m_Velocity(0.1)
{
}

mitk::VirtualTrackingTool::~VirtualTrackingTool()
{
}
