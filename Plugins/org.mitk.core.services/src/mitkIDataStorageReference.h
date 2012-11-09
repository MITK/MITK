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

#ifndef MITKIDATASTORAGEREFERENCE_H_
#define MITKIDATASTORAGEREFERENCE_H_

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
  berryInterfaceMacro(IDataStorageReference, mitk);


  virtual DataStorage::Pointer GetDataStorage() const = 0;

  virtual bool IsDefault() const = 0;

  virtual QString GetLabel() const = 0;
  virtual void SetLabel(const QString& label) = 0;

  virtual bool operator==(const berry::Object* o) const = 0;
};

}

#endif /*MITKIDATASTORAGEREFERENCE_H_*/
