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

#include "mitkNavigationToolStorage.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>

const std::string  mitk::NavigationToolStorage::US_INTERFACE_NAME = "org.mitk.services.NavigationToolStorage"; // Name of the interface
const std::string  mitk::NavigationToolStorage::US_PROPKEY_SOURCE_ID = US_INTERFACE_NAME + ".sourceID";
const std::string  mitk::NavigationToolStorage::US_PROPKEY_STORAGE_NAME = US_INTERFACE_NAME + ".name";

mitk::NavigationToolStorage::NavigationToolStorage() : m_ToolCollection(std::vector<mitk::NavigationTool::Pointer>()),m_DataStorage(NULL),m_storageLocked(false)
  {
  this->SetName("ToolStorage (no name given)");
  }

mitk::NavigationToolStorage::NavigationToolStorage(mitk::DataStorage::Pointer ds) : m_storageLocked(false)
  {
  m_ToolCollection = std::vector<mitk::NavigationTool::Pointer>();
  this->m_DataStorage = ds;
  }

void mitk::NavigationToolStorage::SetName(std::string n)
  {
  m_Name = n;
  m_props[ US_PROPKEY_STORAGE_NAME ] = m_Name;
  }


mitk::NavigationToolStorage::~NavigationToolStorage()
  {
  if (m_DataStorage.IsNotNull()) //remove all nodes from the data storage
    {
    for(std::vector<mitk::NavigationTool::Pointer>::iterator it = m_ToolCollection.begin(); it != m_ToolCollection.end(); it++)
        m_DataStorage->Remove((*it)->GetDataNode());
    }
  }


void mitk::NavigationToolStorage::RegisterAsMicroservice(std::string sourceID){

  if ( sourceID.empty() ) mitkThrow() << "Empty or null string passed to NavigationToolStorage::registerAsMicroservice().";

  // Get Context
  us::ModuleContext* context = us::GetModuleContext();

  // Define ServiceProps
  m_props[ US_PROPKEY_SOURCE_ID ] = sourceID;
  m_ServiceRegistration = context->RegisterService(this, m_props);
}


void mitk::NavigationToolStorage::UnRegisterMicroservice(){
  m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}


bool mitk::NavigationToolStorage::DeleteTool(int number)
  {
    if (m_storageLocked)
    {
      MITK_WARN << "Storage is locked, cannot modify it!";
      return false;
    }

    else if ((unsigned int)number > m_ToolCollection.size())
    {
      MITK_WARN << "Tool no " << number << "doesn't exist, can't delete it!";
      return false;
    }
    std::vector<mitk::NavigationTool::Pointer>::iterator it = m_ToolCollection.begin() + number;
    if(m_DataStorage.IsNotNull())
      m_DataStorage->Remove((*it)->GetDataNode());
    m_ToolCollection.erase(it);

    return true;
  }

bool mitk::NavigationToolStorage::DeleteAllTools()
  {
   if (m_storageLocked)
    {
    MITK_WARN << "Storage is locked, cannot modify it!";
    return false;
    }

  while(m_ToolCollection.size() > 0) if (!DeleteTool(0)) return false;
  return true;
  }

bool mitk::NavigationToolStorage::AddTool(mitk::NavigationTool::Pointer tool)
  {
  if (m_storageLocked)
    {
    MITK_WARN << "Storage is locked, cannot modify it!";
    return false;
    }
  else if (GetTool(tool->GetIdentifier()).IsNotNull())
    {
    MITK_WARN << "Tool ID already exists in storage, can't add!";
    return false;
    }
  else
    {
    m_ToolCollection.push_back(tool);
    if(m_DataStorage.IsNotNull())
      {
      if (!m_DataStorage->Exists(tool->GetDataNode()))
        m_DataStorage->Add(tool->GetDataNode());
      }
    return true;
    }
  }

mitk::NavigationTool::Pointer mitk::NavigationToolStorage::GetTool(int number)
  {
  return m_ToolCollection.at(number);
  }

mitk::NavigationTool::Pointer mitk::NavigationToolStorage::GetTool(std::string identifier)
  {
  for (int i=0; i<GetToolCount(); i++) if ((GetTool(i)->GetIdentifier())==identifier) return GetTool(i);
  return NULL;
  }

mitk::NavigationTool::Pointer mitk::NavigationToolStorage::GetToolByName(std::string name)
  {
  for (int i=0; i<GetToolCount(); i++) if ((GetTool(i)->GetToolName())==name) return GetTool(i);
  return NULL;
  }

int mitk::NavigationToolStorage::GetToolCount()
  {
  return m_ToolCollection.size();
  }

bool mitk::NavigationToolStorage::isEmpty()
  {
  return m_ToolCollection.empty();
  }

void mitk::NavigationToolStorage::LockStorage()
  {
  m_storageLocked = true;
  }

void mitk::NavigationToolStorage::UnLockStorage()
  {
  m_storageLocked = false;
  }

bool mitk::NavigationToolStorage::isLocked()
  {
  return m_storageLocked;
  }
