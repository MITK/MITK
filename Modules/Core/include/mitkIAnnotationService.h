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

#ifndef mitkIAnnotationService_h
#define mitkIAnnotationService_h

#include <mitkServiceInterface.h>
#include <string>
#include <vector>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   * \brief Interface of property aliases service.
   *
   * This service allows you to manage aliases of property names.
   * A property name can be mapped to more than one alias and aliases can be restricted to specific data node types.
   * The property view prefers to display aliases instead of genuine property names.
   */
  class MITKCORE_EXPORT IAnnotationService
  {
  public:
    virtual ~IAnnotationService();

  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IAnnotationService, "org.mitk.IAnnotationService")

#endif
