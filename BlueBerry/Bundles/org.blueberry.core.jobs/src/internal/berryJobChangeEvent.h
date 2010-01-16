/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef BERRYJOBCHANGEEVENT_H_
#define BERRYJOBCHANGEEVENT_H_

#include "../berryIJobChangeEvent.h"

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

  berryObjectMacro(JobChangeEvent)

  JobChangeEvent();

  /* 
   * Method declared on IJobChangeEvent
   */
  Poco::Timestamp::TimeDiff GetDelay() const;

  /* 
   * Method declared on IJobChangeEvent
   */
  SmartPointer<Job> GetJob() const;

  /* (
   * Method declared on IJobChangeEvent
   */
  IStatus::Pointer GetResult() const;

};

}

#endif /* BERRYJOBCHANGEEVENT_H_ */
