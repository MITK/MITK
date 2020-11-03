/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataGenerationJobBase.h"


std::string QmitkDataGenerationJobBase::GetLastErrorMessage() const
{
  return m_LastErrorMessage;
}

bool QmitkDataGenerationJobBase::GetComputationSuccessFlag() const
{
  return m_ComputationSuccessful;
}

void QmitkDataGenerationJobBase::run()
{
  try
  {
    m_ComputationSuccessful = this->RunComputation();
    if (m_ComputationSuccessful)
    {
      emit ResultsAvailable(this->GetResults(), this);
    }
    else
    {
      emit Error(QStringLiteral("Error while running computation. Error description: ") + QString::fromStdString(m_LastErrorMessage), this);
    }
  }
  catch (const std::exception& e)
  {
    m_LastErrorMessage = e.what();
    emit Error(QStringLiteral("Error while running computation. Error description: ") + QString::fromStdString(m_LastErrorMessage), this);

  }
  catch (...)
  {
    m_LastErrorMessage = "Unknown exception";
    emit Error(QStringLiteral("Error while running computation. Error description: ") + QString::fromStdString(m_LastErrorMessage), this);
  }
}
