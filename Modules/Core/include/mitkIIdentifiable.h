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

#ifndef mitkIIdentifiable_h
#define mitkIIdentifiable_h

#include <string>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Interface of identifiable objects.
   */
  class MITKCORE_EXPORT IIdentifiable
  {
  public:
    using UIDType = std::string;
    virtual ~IIdentifiable();

    /**
     * \brief Get unique ID of an object.
     *
     * \return Empty string if an object has no unique ID.
     */
    virtual UIDType GetUID() const = 0;
  };
}

#endif
