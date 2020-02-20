/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIJOBCHANGEEVENT_H_
#define BERRYIJOBCHANGEEVENT_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <berryIStatus.h>

#include <Poco/Timestamp.h>

#include <org_blueberry_core_jobs_Export.h>

namespace berry
{

class Job;

/**
 * An event describing a change to the state of a job.
 *
 * @see IJobChangeListener
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_JOBS IJobChangeEvent : public Object
{

  berryObjectMacro(berry::IJobChangeEvent);

  /**
   * The amount of time in milliseconds to wait after scheduling the job before it
   * should be run, or <code>-1</code> if not applicable for this type of event.
   * This value is only applicable for the <code>scheduled</code> event.
   *
   * @return the delay time for this event
   */
  virtual Poco::Timestamp::TimeDiff GetDelay() const = 0;

  /**
   * The job on which this event occurred.
   *
   * @return the job for this event
   */
  virtual SmartPointer<Job> GetJob() const = 0;

  /**
   * The result returned by the job's run method, or <code>null</code> if
   * not applicable.  This value is only applicable for the <code>done</code> event.
   *
   * @return the status for this event
   */
  virtual IStatus::Pointer GetResult() const = 0;


};

}

#endif /* BERRYIJOBCHANGEEVENT_H_ */
