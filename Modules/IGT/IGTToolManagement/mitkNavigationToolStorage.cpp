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

mitk::NavigationToolStorage::NavigationToolStorage()
  {
  m_ToolCollection = std::vector<mitk::NavigationTool::Pointer>();
  this->m_DataStorage = NULL;
  }

mitk::NavigationToolStorage::NavigationToolStorage(mitk::DataStorage::Pointer ds)
  {
  m_ToolCollection = std::vector<mitk::NavigationTool::Pointer>();
  this->m_DataStorage = ds;
  }



mitk::NavigationToolStorage::~NavigationToolStorage()
  {
  if (m_DataStorage.IsNotNull()) //remove all nodes from the data storage
    {
    for(std::vector<mitk::NavigationTool::Pointer>::iterator it = m_ToolCollection.begin(); it != m_ToolCollection.end(); it++)
        m_DataStorage->Remove((*it)->GetDataNode());
    }
  }

bool mitk::NavigationToolStorage::DeleteTool(int number)
  {
    if ((unsigned int)number > m_ToolCollection.size()) return false;
    std::vector<mitk::NavigationTool::Pointer>::iterator it = m_ToolCollection.begin() + number;
    if(m_DataStorage.IsNotNull())
      m_DataStorage->Remove((*it)->GetDataNode());
    m_ToolCollection.erase(it);

    return true;
  }

bool mitk::NavigationToolStorage::DeleteAllTools()
  {
  while(m_ToolCollection.size() > 0) if (!DeleteTool(0)) return false;
  return true;
  }

bool mitk::NavigationToolStorage::AddTool(mitk::NavigationTool::Pointer tool)
  {
  if (GetTool(tool->GetIdentifier()).IsNotNull()) return false;
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
