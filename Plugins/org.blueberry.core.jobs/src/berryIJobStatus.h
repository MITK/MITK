/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef _BERRYIJOBSTATUS_H
#define _BERRYIJOBSTATUS_H

#include "berryIStatus.h"
#include "berryObject.h"
#include "berryJob.h"

 namespace berry {

 /**
  * Represents status relating to the execution of jobs.
  * @see IStatus
  * @noimplement This interface is not intended to be implemented by clients.
  * @noextend This interface is not intended to be extended by clients.
  */

struct IJobStatus : public virtual IStatus
{

   berryObjectMacro(IJobStatus);

  /**
   * Returns the job associated with this status.
   * @return the job associated with this status
   */
    virtual Job::Pointer GetJob() = 0;
  };
}


#endif /*_BERRYIJOBSTATUS_H */
