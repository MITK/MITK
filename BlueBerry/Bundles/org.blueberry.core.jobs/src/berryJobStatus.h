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

#ifndef _BERRYJOBSTATUS_H
#define _BERRYJOBSTATUS_H  

#include "berryIJobStatus.h"
#include "berryStatus.h"
#include "berryJob.h"
#include <string>

namespace berry {

  class BERRY_JOBS JobStatus : public IJobStatus {

     
  public:

  berryObjectMacro(JobStatus)

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
   *  @see org.blueberry.solstice.common IStatus GetChildren() 
   */
   std::vector<IStatus::Pointer> GetChildren() const ;

  /**
   * @see org.blueberry.solstice.common IStatus GetCode()
   */
   int GetCode() const ;

  /**
   *@see org.blueberry.solstice.common IStatus GetException ()
   */
   std::exception GetException() const ;

  /**
   * @see org.blueberry.solstice.common GetMessage ()
   */
   std::string GetMessage() const ;

  /**
   * @see org.blueberry.solstice.common IStatus GetPlugin()
   */
   std::string GetPlugin() const ;

  /**
   * @see org.blueberry.solstice.common IStatus GetSeverity () 
   */
   Severity GetSeverity() const ;

  /**
   * @see org.blueberry.solstice.common IsMultiStatus ()
   */
   bool IsMultiStatus() const ;

  /**
   * @see org.blueberry.solstice.common IStatus IsOk ()
   */
   bool IsOK() const ;

  /**
   * @see org.blueberry.solstice.common IStatus Matches ()
   */
   bool Matches(const Severities& severityMask) const ;


  private: 

    Job::Pointer m_myJob ; 

    Status::Pointer m_internalStatus ;


  };


 }

#endif /* _BERRYJOBSTATUS_H */