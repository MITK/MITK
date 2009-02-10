/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTimeStamp.h"

mitk::TimeStamp::TimeStamp() : itk::Object()
, m_Time(0.0)
{
}

mitk::TimeStamp::~TimeStamp()
{
}

const mitk::TimeStamp* mitk::TimeStamp::GetTimeStamp()
{
  return new TimeStamp();  // \TODO Access real time clock to get meaningful timestamp
}
