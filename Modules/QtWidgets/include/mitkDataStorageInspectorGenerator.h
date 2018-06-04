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

#ifndef mitkDataStorageInspectorGenerator_H
#define mitkDataStorageInspectorGenerator_H

#include "mitkIDataStorageInspectorProvider.h"

#include <MitkQtWidgetsExports.h>

namespace mitk
{
  /** Convinvience class to get all or specific DataStorageInspectorProvider. */
  class MITKQTWIDGETS_EXPORT DataStorageInspectorGenerator
  {
  public:
    using IDType = std::string;

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
