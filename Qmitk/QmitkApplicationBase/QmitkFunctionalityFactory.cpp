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

#include "QmitkFunctionalityFactory.h"

QmitkFunctionalityFactory& QmitkFunctionalityFactory::GetInstance() {
  static QmitkFunctionalityFactory* instance = new QmitkFunctionalityFactory();
  return *instance;  
}; 
QmitkFunctionalityFactory::CreateFunctionalityPtr QmitkFunctionalityFactory::GetCreateFunctionalityPtrByName(const std::string name) const {
  CreateFunctionalityPtrMap::const_iterator it = m_RegisteredFunctionalities.find(name);
  if ( it != m_RegisteredFunctionalities.end()) {
    return it->second;
  }
  else
  {
    return NULL;
  }
}    

