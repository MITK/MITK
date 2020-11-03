/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYINVALIDREGISTRYOBJECTEXCEPTION_H
#define BERRYINVALIDREGISTRYOBJECTEXCEPTION_H

#include <ctkException.h>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

/**
 * An exception indicating an attempt to access
 * an extension registry object that is no longer valid.
 * <p>
 * This exception is thrown by methods on extension registry
 * objects. It is not intended to be instantiated or
 * subclassed by clients.
 * </p>
 * @noinstantiate This class is not intended to be instantiated by clients.
 * @noextend This class is not intended to be subclassed by clients.
 */
class org_blueberry_core_runtime_EXPORT InvalidRegistryObjectException : public ctkRuntimeException
{
public:

  InvalidRegistryObjectException();
  ~InvalidRegistryObjectException() throw() override;

  const char* name() const throw() override;
  InvalidRegistryObjectException* clone() const override;
  void rethrow() const override;
};

}

#endif // BERRYINVALIDREGISTRYOBJECTEXCEPTION_H
