/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

