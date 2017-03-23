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

#include <mitkPolhemusInterface.h>

#include <windows.h>
#include <tchar.h>
#include <string>
#include <PDI.h>


mitk::PolhemusInterface::PolhemusInterface()
{

}

mitk::PolhemusInterface::~PolhemusInterface()
{

}

bool mitk::PolhemusInterface::StartTracking()
{

  return true;
}

bool mitk::PolhemusInterface::StopTracking()
{

  return true;

}
