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

#ifndef mitkGetPropertyService_h
#define mitkGetPropertyService_h

#include "org_mitk_gui_qt_properties_Activator.h"

namespace mitk
{
  template <class T>
  T* GetPropertyService()
  {
    ctkPluginContext* context = mitk::org_mitk_gui_qt_properties_Activator::GetContext();
    ctkServiceReference serviceRef = context->getServiceReference<T>();

    return serviceRef
      ? context->getService<T>(serviceRef)
      : NULL;
  }
}

#endif
