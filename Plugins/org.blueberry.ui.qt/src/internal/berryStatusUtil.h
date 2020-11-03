/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
