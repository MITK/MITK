/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGetPropertyService_h
#define mitkGetPropertyService_h

#include "org_mitk_gui_qt_overlaymanager_Activator.h"

namespace mitk
{
  template <class T>
  T* GetPropertyService()
  {
    ctkPluginContext* context = mitk::org_mitk_gui_qt_overlaymanager_Activator::GetContext();
    ctkServiceReference serviceRef = context->getServiceReference<T>();

    return serviceRef
      ? context->getService<T>(serviceRef)
      : nullptr;
  }
}

#endif
