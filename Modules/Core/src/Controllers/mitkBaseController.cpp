/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBaseController.h"
#include "mitkBaseRenderer.h"

mitk::BaseController::BaseController()
  : m_LastUpdateTime(0)
{
  m_Stepper = Stepper::New();
}

mitk::BaseController::~BaseController()
{
}

void mitk::BaseController::ExecuteOperation(mitk::Operation * /* *operation */)
{
}

mitk::Stepper *mitk::BaseController::GetStepper()
{
  return m_Stepper.GetPointer();
}

const mitk::Stepper* mitk::BaseController::GetStepper() const
{
  return m_Stepper.GetPointer();
}

