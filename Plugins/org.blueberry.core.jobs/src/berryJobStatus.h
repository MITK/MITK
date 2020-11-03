/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _BERRYJOBSTATUS_H
#define _BERRYJOBSTATUS_H

#include "berryIJobStatus.h"
#include "berryStatus.h"
#include "berryJob.h"

namespace berry {

class BERRY_JOBS JobStatus : public Status, public IJobStatus
{

public:

  berryObjectMacro(JobStatus);

  /**
   * Creates a new job status with no interesting error code or exception.
   * @param severity
   * @param job
   * @param message
   */
  JobStatus(const Severity& severity, Job::Pointer sptr_job, const QString& message,
            const Status::SourceLocation& sl);

  /**
   * @see IJobStatus#GetJob()
   */
  Job::Pointer GetJob() override;

private:

  Job::Pointer m_myJob;

};


}

#endif /* _BERRYJOBSTATUS_H */
