/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

const mitk::Stepper* mitk::BaseController::GetSlice() const
{
  return m_Slice.GetPointer();
}

mitk::Stepper *mitk::BaseController::GetTime()
{
  return m_Time.GetPointer();
}

const mitk::Stepper* mitk::BaseController::GetTime() const
{
  return m_Time.GetPointer();
}
