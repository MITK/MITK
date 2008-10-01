/*=========================================================================

Program:   openCherry Platform
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

#include "mitkDataStorageEditorInput.h"

#include <cherryPlatform.h>
#include <mitkIDataStorageService.h>

namespace mitk
{

bool DataStorageEditorInput::Exists() const
{
  return true;
}

std::string DataStorageEditorInput::GetName() const
{
  return "DataStorage Scene";
}

std::string DataStorageEditorInput::GetToolTipText() const
{
  return "";
}

bool DataStorageEditorInput::operator==(const cherry::IEditorInput* o) const
{
  const DataStorageEditorInput* input = dynamic_cast<const DataStorageEditorInput*>(o);
  if (input == 0) return false;

  return this->GetName() == input->GetName();
}

IDataStorageReference::Pointer
DataStorageEditorInput::GetDataStorageReference()
{
  if (m_DataStorageRef.IsNull())
  {
    cherry::ServiceRegistry& serviceRegistry = cherry::Platform::GetServiceRegistry();
    IDataStorageService::Pointer dataService = serviceRegistry.GetServiceById<IDataStorageService>(IDataStorageService::ID);
    if (dataService.IsNull()) return 0;
    m_DataStorageRef = dataService->GetDefaultDataStorage();
  }

  return m_DataStorageRef;
}

}
