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

#include "mitkGetDataStorage.h"
#include "org_mitk_simulation_Activator.h"
#include <mitkIDataStorageService.h>

mitk::DataStorage::Pointer mitk::GetDataStorage()
{
  IDataStorageService* dataStorageService = org_mitk_simulation_Activator::GetService<mitk::IDataStorageService>();

  if (dataStorageService != NULL)
  {
    IDataStorageReference::Pointer dataStorageReference = dataStorageService->GetDefaultDataStorage();

    if (dataStorageReference.IsNotNull())
      return dataStorageReference->GetDataStorage();
  }

  return NULL;
}
