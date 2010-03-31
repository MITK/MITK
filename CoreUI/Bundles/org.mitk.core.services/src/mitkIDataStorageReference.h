/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKIDATASTORAGEREFERENCE_H_
#define MITKIDATASTORAGEREFERENCE_H_

#include <berryObject.h>
#include <berryMacros.h>

#include "mitkCoreServicesPluginDll.h"

#include <mitkDataStorage.h>

namespace mitk
{

struct MITK_CORE_SERVICES_PLUGIN IDataStorageReference : public berry::Object
{
  berryInterfaceMacro(IDataStorageReference, mitk);


  virtual DataStorage::Pointer GetDataStorage() const = 0;

  virtual bool IsDefault() const = 0;

  virtual std::string GetLabel() const = 0;
  virtual void SetLabel(const std::string& label) = 0;

  virtual bool operator==(const IDataStorageReference* o) const = 0;
};

}

#endif /*MITKIDATASTORAGEREFERENCE_H_*/
