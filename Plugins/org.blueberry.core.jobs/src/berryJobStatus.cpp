/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryJobStatus.h"
#include "internal/berryJobManager.h"
#include <org_blueberry_core_jobs_Export.h>

namespace berry
{

JobStatus::JobStatus(const Severity& severity, Job::Pointer sptr_job,
                     const QString& message, const Status::SourceLocation& sl)
  : Status(severity, JobManager::PI_JOBS(), 1, message, sl)
  , m_myJob(sptr_job)
{
}

Job::Pointer JobStatus::GetJob()
{
  return m_myJob;
}

}

