/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorageInspectorGenerator_H
#define mitkDataStorageInspectorGenerator_H

#include "mitkIDataStorageInspectorProvider.h"

#include <MitkQtWidgetsExports.h>

namespace mitk
{
  /** Convenience class to get all or specific DataStorageInspectorProvider. */
  class MITKQTWIDGETS_EXPORT DataStorageInspectorGenerator
  {
  public:
    using IDType = mitk::IDataStorageInspectorProvider::InspectorIDType;

    using ProviderMapType = std::map<IDType, mitk::IDataStorageInspectorProvider*>;
    static ProviderMapType GetProviders();

    static mitk::IDataStorageInspectorProvider *GetProvider(const IDType &id);

  protected:
    DataStorageInspectorGenerator();
    virtual ~DataStorageInspectorGenerator();
    DataStorageInspectorGenerator(const DataStorageInspectorGenerator &source) = delete;
    DataStorageInspectorGenerator& operator=(const DataStorageInspectorGenerator &) = delete;
  };
}

#endif
