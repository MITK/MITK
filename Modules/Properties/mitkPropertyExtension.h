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

#ifndef mitkPropertyExtension_h
#define mitkPropertyExtension_h

#include <PropertiesExports.h>

namespace mitk
{
  class Properties_EXPORT PropertyExtension
  {
  public:
    PropertyExtension();
    virtual ~PropertyExtension();

  private:
    PropertyExtension(const PropertyExtension& other);
    PropertyExtension& operator=(PropertyExtension other);
  };
}

#endif
