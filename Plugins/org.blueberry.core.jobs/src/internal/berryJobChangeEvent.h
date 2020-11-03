/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYJOBCHANGEEVENT_H_
#define BERRYJOBCHANGEEVENT_H_

#include "berryIJobChangeEvent.h"

namespace berry {

class JobChangeEvent : public IJobChangeEvent
{

private:

  friend class JobListeners;

  /**
   * The job on which this event occurred.
   */
  SmartPointer<Job> job;

  /**
   * The result returned by the job's run method, or <code>null</code> if
   * not applicable.
   */
  IStatus::Pointer result;

  /**
   * The amount of time to wait after scheduling the job before it should be run,
   * or <code>-1</code> if not applicable for this type of event.
   */
  Poco::Timestamp::TimeDiff delay;

  /**
   * Whether this job is being immediately rescheduled.
   */
  bool reschedule;

public:

  berryObjectMacro(JobChangeEvent);

  JobChangeEvent();

  /*
   * Method declared on IJobChangeEvent
   */
  Poco::Timestamp::TimeDiff GetDelay() const override;

  /*
   * Method declared on IJobChangeEvent
   */
  SmartPointer<Job> GetJob() const override;

  /* (
   * Method declared on IJobChangeEvent
   */
  IStatus::Pointer GetResult() const override;

};

}

#endif /* BERRYJOBCHANGEEVENT_H_ */
