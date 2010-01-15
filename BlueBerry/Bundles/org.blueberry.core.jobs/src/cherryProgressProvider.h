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

#ifndef _CHERRY_PROGRESSPROVIDER_H
#define _CHERRY_PROGRESSPROVIDER_H

#include "cherryIProgressMonitor.h"
#include "cherryObject.h"
#include "cherryJob.h"

namespace cherry
{

/**
 * The progress provider supplies the job manager with progress monitors for
 * running jobs.  There can only be one progress provider at any given time.
 * <p>
 * This class is intended for use by the currently executing Eclipse application.
 * Plug-ins outside the currently running application should not reference or 
 * subclass this class.
 * </p>
 * 
 * @see IJobManager#SetProgressProvider(ProgressProvider::Pointer)
 */
struct ProgressProvider: public Object
{

public:

  cherryObjectMacro(ProgressProvider)

  /**
   * Provides a new progress monitor instance to be used by the given job.
   * This method is called prior to running any job that does not belong to a
   * progress group. The returned monitor will be supplied to the job's
   * <code>Run</code> method.
   *
   * @see #CreateProgressGroup()
   * @see Job#SetProgressGroup(IProgressMonitor::Pointer, int)
   * @param job the job to create a progress monitor for
   * @return a progress monitor, or <code>null</code> if no progress monitoring
   * is needed.
   */
  virtual IProgressMonitor::Pointer CreateMonitor(Job::Pointer job) = 0;

  /**
   * Returns a progress monitor that can be used to provide
   * aggregated progress feedback on a set of running jobs.
   * This method implements <code>IJobManager.createProgressGroup</code>,
   * and must obey all rules specified in that contract.
   * <p>
   * This default implementation returns a new
   * <code>NullProgressMonitor</code>  Subclasses may override.
   *
   * @see IJobManager#CreateProgressGroup()
   * @return a progress monitor
   */
  virtual IProgressMonitor::Pointer CreateProgressGroup();

  /**
   * Returns a progress monitor that can be used by a running job
   * to report progress in the context of a progress group. This method
   * implements <code>Job.setProgressGroup</code>.  One of the
   * two <code>createMonitor</code> methods will be invoked
   * prior to each execution of a job, depending on whether a progress
   * group was specified for the job.
   * <p>
   * The provided monitor must be a monitor returned by the method
   * <code>createProgressGroup</code>.  This method is responsible
   * for asserting this and throwing an appropriate runtime exception
   * if an invalid monitor is provided.
   * <p>
   * This default implementation returns a new
   * <code>SubProgressMonitor</code>.  Subclasses may override.
   *
   * @see IJobManager#CreateProgressGroup()
   * @see Job#setProgressGroup(IProgressMonitor, int)
   * @param job the job to create a progress monitor for
   * @param group the progress monitor group that this job belongs to
   * @param ticks the number of ticks of work for the progress monitor
   * @return a progress monitor, or <code>null</code> if no progress monitoring
   * is needed.
   */
  virtual IProgressMonitor::Pointer CreateMonitor(Job::Pointer sptr_job,
      IProgressMonitor::Pointer sptr_group, int ticks);

  /**
   * Returns a progress monitor to use when none has been provided
   * by the client running the job.
   * <p>
   * This default implementation returns a new
   * <code>NullProgressMonitor</code>  Subclasses may override.
   *
   * @return a progress monitor
   */
  virtual IProgressMonitor::Pointer GetDefaultMonitor();
};

}
#endif /* CHERRY_PROGRESSPROVIDER_H */ 
