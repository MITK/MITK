/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKTESTNOTRUNEXCEPTION_H
#define MITKTESTNOTRUNEXCEPTION_H

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

#endif // MITKTESTNOTRUNEXCEPTION_H
