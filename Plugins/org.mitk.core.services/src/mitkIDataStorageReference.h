/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIDataStorageReference_h
#define mitkIDataStorageReference_h

#include <berryObject.h>
#include <berryMacros.h>

#include <org_mitk_core_services_Export.h>

#include <mitkDataStorage.h>

namespace mitk
{

/**
 * \ingroup org_mitk_core_services
 */
struct MITK_CORE_SERVICES_PLUGIN IDataStorageReference : public berry::Object
{
  berryObjectMacro(mitk::IDataStorageReference);

  virtual DataStorage::Pointer GetDataStorage() const = 0;

  virtual bool IsDefault() const = 0;

  virtual QString GetLabel() const = 0;
  virtual void SetLabel(const QString& label) = 0;

  bool operator==(const berry::Object* o) const override = 0;
};

}

#endif
