/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTestNotRunException_h
#define mitkTestNotRunException_h

#include <mitkException.h>
#include <mitkCommon.h>
#include <MitkTestingHelperExports.h>


namespace mitk
{
  /**Documentation
  * \brief Specialized mitk::Exception for skipped tests
  *
  * If a test is unable to run due to circumstances that do not indicate
  * a failure (such as a missing openGl for a rendering test) it should throw
  * this exception.
  *
  * Usage:
  *       mitkThrowException(mitk::TestNotRunException) << "optional reason for skip";
  */
  class MITKTESTINGHELPER_EXPORT TestNotRunException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(TestNotRunException, mitk::Exception);
  };
}

#endif
