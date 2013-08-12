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

#ifndef MITK_TOOLMANAGERPROVIDER_H
#define MITK_TOOLMANAGERPROVIDER_H

#include <usServiceInterface.h>

namespace mitk {


  /**
  * \ingroup MicroServices_Interfaces
  *
  */
  struct ToolManagerProvider
  {
    virtual ~ToolManagerProvider() {}

    virtual bool Dummy() = 0;

  };

}


  US_DECLARE_SERVICE_INTERFACE(mitk::ToolManagerProvider, "org.mitk.services.ToolManagerProvider")

#endif
