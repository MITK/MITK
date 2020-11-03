/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkToolCommand.h"
#include "mitkProgressBar.h"

mitk::ToolCommand::ToolCommand() : m_ProgressValue(0), m_StopProcessing(false)
{
}

void mitk::ToolCommand::Execute(itk::Object *, const itk::EventObject &event)
{
  if (typeid(event) == typeid(itk::IterationEvent))
  {
    // MITK_INFO << "IterationEvent";
  }

  if (typeid(event) == typeid(itk::ProgressEvent))
  {
    // MITK_INFO << "ToolCommand::ProgressEvent";
  }

  if (typeid(event) == typeid(itk::AnyEvent))
  {
    // MITK_INFO << "AnyEvent";
  }

  if (typeid(event) == typeid(itk::StartEvent))
  {
    // MITK_INFO << "StartEvent";
  }

  if (typeid(event) == typeid(itk::EndEvent))
  {
    // MITK_INFO << "EndEvent";
  }

  if (typeid(event) == typeid(itk::FunctionEvaluationIterationEvent))
  {
    // MITK_INFO << "FunctionEvaluationIterationEvent";
  }

  if (typeid(event) == typeid(itk::GradientEvaluationIterationEvent))
  {
    // MITK_INFO << "GradientEvaluationIterationEvent";
  }

  if (typeid(event) == typeid(itk::FunctionAndGradientEvaluationIterationEvent))
  {
    // MITK_INFO << "FunctionAndGradientEvaluationIterationEvent";
  }

  mitk::ProgressBar::GetInstance()->Progress();
}

void mitk::ToolCommand::Execute(const itk::Object * /*caller*/, const itk::EventObject & /*event*/)
{
}

void mitk::ToolCommand::AddStepsToDo(int steps)
{
  mitk::ProgressBar::GetInstance()->AddStepsToDo(steps);
}

void mitk::ToolCommand::SetProgress(int steps)
{
  mitk::ProgressBar::GetInstance()->Progress(steps);
}

double mitk::ToolCommand::GetCurrentProgressValue()
{
  return m_ProgressValue;
}

void mitk::ToolCommand::SetStopProcessing(bool value)
{
  m_StopProcessing = value;
}
