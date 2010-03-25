/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-04-02 17:14:50 +0200 (Do, 02. Apr 2009) $
Version:   $Revision: 16785 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkVirtualTrackingTool.h"
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;



typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::VirtualTrackingTool::VirtualTrackingTool() 
: InternalTrackingTool(), m_Spline(NULL), m_SplineLength(0.0), m_Velocity(0.1)
{
  m_Spline = SplineType::New();
}


mitk::VirtualTrackingTool::~VirtualTrackingTool()
{
}


