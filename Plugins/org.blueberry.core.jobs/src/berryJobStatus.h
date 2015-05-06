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
  Job::Pointer GetJob();

private:

  Job::Pointer m_myJob;

};


}

#endif /* _BERRYJOBSTATUS_H */
