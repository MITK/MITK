/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 16492 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryJobStatus.h"
#include "internal/cherryJobManager.h"
#include "cherryJobsDll.h"

namespace cherry
{

JobStatus::JobStatus(const Status::Severity& serverity, Job::Pointer sptr_job,
    const std::string& message) :
  m_myJob(sptr_job), m_internalStatus(new Status(serverity,
      JobManager::PI_JOBS, 1, message))
{
}

Job::Pointer JobStatus::GetJob()
{
  return m_myJob;
}

std::vector<IStatus::Pointer> JobStatus::GetChildren() const
{
  return m_internalStatus->GetChildren();
}

int JobStatus::GetCode() const
{
  return m_internalStatus->GetCode();
}

std::exception JobStatus::GetException() const
{
  return m_internalStatus->GetException();
}

std::string JobStatus::GetMessage() const
{
  return m_internalStatus->GetMessage();
}

std::string JobStatus::GetPlugin() const
{
  return m_internalStatus->GetPlugin();
}

IStatus::Severity JobStatus::GetSeverity() const
{
  return m_internalStatus->GetSeverity();
}

bool JobStatus::IsMultiStatus() const
{
  return m_internalStatus->IsMultiStatus();
}

bool JobStatus::IsOK() const
{
  return m_internalStatus->IsOK();
}

bool JobStatus::Matches(const Severities& severityMask) const
{
  return m_internalStatus->Matches(severityMask);
}

}

