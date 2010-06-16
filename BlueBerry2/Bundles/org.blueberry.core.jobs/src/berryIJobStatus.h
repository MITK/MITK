/*=========================================================================

 Program:   BlueBerry Platform
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

   struct IJobStatus : public IStatus {
  
   berryObjectMacro(IJobStatus) 

  /**
   * Returns the job associated with this status.
   * @return the job associated with this status
   */
    virtual Job::Pointer GetJob() = 0;
  };
}


#endif /*_BERRYIJOBSTATUS_H */ 