/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkPropertyManager.h"
#include "mitkProperties.h"


mitk::PropertyManager::PropertyManager() {
  std::cout << "ctor  PropertyManager" << std::endl;
  m_DefaultPropertyNameSet.insert(std::string("visible"));
}

const mitk::PropertyManager::PropertyNameSet& mitk::PropertyManager::GetDefaultPropertyNames() {
  static mitk::PropertyManager propManager;
  return propManager.m_DefaultPropertyNameSet;
};
mitk::BaseProperty::Pointer mitk::PropertyManager::CreateDefaultProperty(std::string name) {
  mitk::BaseProperty::Pointer newProperty(NULL); 
  if ( name == "visible" ) {
    newProperty = new mitk::BoolProperty(true);
  } else {
    std::cout << "Warning: non-existing default property requested: " 
      << name << std::endl; 
  }  
  return newProperty;
};
