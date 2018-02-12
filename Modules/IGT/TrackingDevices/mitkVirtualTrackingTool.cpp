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
