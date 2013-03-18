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

#ifndef MITKCORESERVICES_H
#define MITKCORESERVICES_H

namespace mitk {

struct IShaderRepository;

/**
 * @brief Access MITK core services.
 *
 * This class can be used inside the MITK Core to conveniently access
 * common service objects.
 *
 * It is not exported and not meant to be used by other MITK modules.
 */
class CoreServices
{
public:

  static IShaderRepository* GetShaderRepository();

private:

  // purposely not implemented
  CoreServices();
  CoreServices(const CoreServices&);
  CoreServices& operator=(const CoreServices&);
};

}

#endif // MITKCORESERVICES_H
