/*=========================================================================

 Program:   openCherry Platform
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

#ifndef _CHERRYJOBSTATUS_H
#define _CHERRYJOBSTATUS_H  

#include "cherryIJobStatus.h"
#include "cherryStatus.h"
#include "cherryJob.h"
#include <string>

namespace cherry {

  class CHERRY_JOBS JobStatus : public IJobStatus {

     
  public:

  cherryObjectMacro(JobStatus)

	/**
	 * Creates a new job status with no interesting error code or exception.
	 * @param severity
	 * @param job
	 * @param message
	 */ 
   JobStatus( const Status::Severity& severity, Job::Pointer sptr_job, const std::string& message) ;  
	
	/** 
	 * @see IJobStatus#GetJob()
	 */
   Job::Pointer GetJob() ;

  /**
   *  @see org.opencherry.solstice.common IStatus GetChildren() 
   */
   std::vector<IStatus::Pointer> GetChildren() const ;

  /**
   * @see org.opencherry.solstice.common IStatus GetCode()
   */
   int GetCode() const ;

  /**
   *@see org.opencherry.solstice.common IStatus GetException ()
   */
   std::exception GetException() const ;

  /**
   * @see org.opencherry.solstice.common GetMessage ()
   */
   std::string GetMessage() const ;

  /**
   * @see org.opencherry.solstice.common IStatus GetPlugin()
   */
   std::string GetPlugin() const ;

  /**
   * @see org.opencherry.solstice.common IStatus GetSeverity () 
   */
   Severity GetSeverity() const ;

  /**
   * @see org.opencherry.solstice.common IsMultiStatus ()
   */
   bool IsMultiStatus() const ;

  /**
   * @see org.opencherry.solstice.common IStatus IsOk ()
   */
   bool IsOK() const ;

  /**
   * @see org.opencherry.solstice.common IStatus Matches ()
   */
   bool Matches(const Severities& severityMask) const ;


  private: 

    Job::Pointer m_myJob ; 

    Status::Pointer m_internalStatus ;


	};


 }

#endif /* _CHERRYJOBSTATUS_H */