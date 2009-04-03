/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14081 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#include "mitkDelegateManager.h"

mitk::DelegateManager::DelegateManager()
{
}
mitk::DelegateManager* mitk::DelegateManager::GetInstance()
{
  static mitk::DelegateManager _DelegateManager;
  return &_DelegateManager;
}

void mitk::DelegateManager::SetCommand( const std::string& _CommandID, mitk::MessageAbstractDelegate* _Command)
{
  MessageAbstractDelegate* _ExistingCommand = this->GetCommand(_CommandID);
  // delete existing MessageAbstractDelegate for id, but only if the existing MessageAbstractDelegate is not the same as the
  // MessageAbstractDelegate parameter
  if(_ExistingCommand && _ExistingCommand != _Command)
  {
    m_CommandMap.erase(_CommandID);
    delete _ExistingCommand;
    _ExistingCommand = 0;
  }
  m_CommandMap[_CommandID] = _Command;
}

bool mitk::DelegateManager::RemoveCommand( const std::string& _CommandID )
{
  MessageAbstractDelegate* _ExistingCommand = this->GetCommand(_CommandID);
  if(_ExistingCommand)
  {
    m_CommandMap.erase(_CommandID);
    delete _ExistingCommand;
    _ExistingCommand = 0;
    return true;
  }
  return false;
}

mitk::MessageAbstractDelegate* mitk::DelegateManager::GetCommand( const std::string& _CommandID ) const
{
  std::map<std::string, MessageAbstractDelegate*>::const_iterator it = m_CommandMap.find(_CommandID);
  return (it == m_CommandMap.end()) ? 0: it->second;
}

mitk::DelegateManager::~DelegateManager()
{
  // delete all commands
  for(std::map<std::string, MessageAbstractDelegate*>::iterator it=m_CommandMap.begin(); it!=m_CommandMap.end()
    ; it++)
  {
    delete it->second;
    it->second = 0;
  }

  m_CommandMap.clear();
}