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

#ifndef QmitkDataStorageInspectorGenerator_H
#define QmitkDataStorageInspectorGenerator_H

#include "QmitkIDataStorageInspectorProvider.h"

#include <MitkQtWidgetsExports.h>

class MITKQTWIDGETS_EXPORT QmitkDataStorageInspectorGenerator
{
public:
  using IDType = std::string;

  using ProviderMapType = std::map<IDType, mitk::IDataStorageInspectorProvider*>;
  static ProviderMapType GetProviders();

  static mitk::IDataStorageInspectorProvider *GetProvider(const IDType &id);

protected:
  QmitkDataStorageInspectorGenerator();
  virtual ~QmitkDataStorageInspectorGenerator();

private:
  // No copy constructor allowed
  QmitkDataStorageInspectorGenerator(const QmitkDataStorageInspectorGenerator &source);
  void operator=(const QmitkDataStorageInspectorGenerator &); // purposely not implemented
};

#endif
