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

#ifndef BERRYSTATUSUTIL_H
#define BERRYSTATUSUTIL_H

#include "berryStatus.h"

namespace berry {

/**
 * Utility class to create status objects.
 *
 * @private - This class is an internal implementation class and should
 * not be referenced or subclassed outside of the workbench
 */
class StatusUtil
{

public:

  /**
   * This method must not be called outside the workbench.
   *
   * Utility method for creating status.
   */
  static IStatus::Pointer NewStatus(IStatus::Severity severity, const QString& message,
                                    const Status::SourceLocation& sl);

  /**
   * This method must not be called outside the workbench.
   *
   * Utility method for creating status.
   */
  static IStatus::Pointer NewStatus(IStatus::Severity severity, const QString& message,
                                    const ctkException& exc, const Status::SourceLocation &sl);

private:

  static ctkException GetCause(const ctkException& exc);

};

}

#endif // BERRYSTATUSUTIL_H
