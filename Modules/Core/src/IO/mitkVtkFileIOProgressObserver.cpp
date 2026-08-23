/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtkFileIOProgressObserver.h"

#include <vtkAlgorithm.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

mitk::VtkFileIOProgressObserver::VtkFileIOProgressObserver(vtkAlgorithm* algorithm,
                                                           std::function<void(float)> report)
  : m_Algorithm(algorithm),
    m_Command(vtkSmartPointer<vtkCallbackCommand>::New()),
    m_ObserverTag(0),
    m_Report(std::move(report))
{
  if (nullptr == m_Algorithm || !m_Report)
  {
    m_Algorithm = nullptr;
    return;
  }

  m_Command->SetClientData(this);
  m_Command->SetCallback(&VtkFileIOProgressObserver::OnProgress);

  m_ObserverTag = m_Algorithm->AddObserver(vtkCommand::ProgressEvent, m_Command);
}

mitk::VtkFileIOProgressObserver::~VtkFileIOProgressObserver()
{
  if (nullptr != m_Algorithm)
    m_Algorithm->RemoveObserver(m_ObserverTag);
}

void mitk::VtkFileIOProgressObserver::OnProgress(vtkObject* caller, unsigned long, void* clientData, void*)
{
  auto* observer = static_cast<VtkFileIOProgressObserver*>(clientData);
  auto* algorithm = vtkAlgorithm::SafeDownCast(caller);

  if (nullptr == observer || nullptr == algorithm)
    return;

  observer->m_Report(static_cast<float>(algorithm->GetProgress()));
}
