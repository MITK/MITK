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

#ifndef BERRYCOREEXCEPTION_H
#define BERRYCOREEXCEPTION_H

#include <berryIStatus.h>

#include <ctkException.h>

namespace berry {

struct IStatus;

/**
 * A checked exception representing a failure.
 * <p>
 * Core exceptions contain a status object describing the cause of the exception.
 * </p><p>
 * This class can be used without OSGi running.
 * </p>
 * @see IStatus
 */
class org_blueberry_core_runtime_EXPORT CoreException : public ctkException
{

public:

  /**
   * Creates a new exception with the given status object.  The message
   * of the given status is used as the exception message.
   *
   * @param status the status object to be associated with this exception
   */
  CoreException(const SmartPointer<IStatus>& status);

  ~CoreException() throw() override;

  const char* name() const throw() override;

  CoreException* clone() const override;

  void rethrow() const override;

  /**
   * Returns the status object for this exception.
   *
   * @return a status object
   */
  SmartPointer<IStatus> GetStatus() const;

private:

  /** Status object. */
  const SmartPointer<IStatus> status;

  /**
   * Prints a stack trace out for the exception, and
   * any nested exception that it may have embedded in
   * its Status object.
   *
   * @param output the stream to write to
   */
  QDebug printStackTrace(QDebug dbg) const override;

  QDebug PrintChildren(IStatus* status, QDebug dbg) const;
};

}

#endif // BERRYCOREEXCEPTION_H
