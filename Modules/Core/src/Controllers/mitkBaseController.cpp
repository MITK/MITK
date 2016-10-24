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

#include "mitkBaseController.h"
#include "mitkBaseRenderer.h"

mitk::BaseController::BaseController() : m_LastUpdateTime(0)
{
  m_Slice = Stepper::New();
  m_Time = Stepper::New();
}

mitk::BaseController::~BaseController()
{
}

void mitk::BaseController::ExecuteOperation(mitk::Operation * /* *operation */)
{
}

mitk::Stepper *mitk::BaseController::GetSlice()
{
  return m_Slice.GetPointer();
}

mitk::Stepper *mitk::BaseController::GetTime()
{
  return m_Time.GetPointer();
}
