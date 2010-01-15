/*=========================================================================

 Program:   openCherry Platform
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
#ifndef _CHERRYIJOBSTATUS_H  
#define _CHERRYIJOBSTATUS_H

#include "cherryIStatus.h"
#include "cherryObject.h"
#include "cherryJob.h"

 namespace cherry {

 /**
  * Represents status relating to the execution of jobs.
  * @see IStatus
  * @noimplement This interface is not intended to be implemented by clients.
  * @noextend This interface is not intended to be extended by clients.
  */

   struct IJobStatus : public IStatus {
  
   cherryObjectMacro(IJobStatus) 

	/**
	 * Returns the job associated with this status.
	 * @return the job associated with this status
	 */
    virtual Job::Pointer GetJob() = 0;
  };
}


#endif /*_CHERRYIJOBSTATUS_H */ 