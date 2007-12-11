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


#include "mitkPropertyManager.h"
#include "mitkProperties.h"
#include <limits>

mitk::PropertyManager::PropertyManager()
{
  m_DefaultPropertyNameSet.insert(std::string("visible"));
  m_PropertyLimits.insert(std::make_pair("opacity",std::make_pair(0.0f,1.0f)));
}
mitk::PropertyManager* mitk::PropertyManager::GetInstance()
{
  static mitk::PropertyManager propManager;
  return &propManager;
}
const mitk::PropertyManager::PropertyNameSet& mitk::PropertyManager::GetDefaultPropertyNames()
{
  return m_DefaultPropertyNameSet;
};
mitk::BaseProperty::Pointer mitk::PropertyManager::CreateDefaultProperty(std::string name)
{
  mitk::BaseProperty::Pointer newProperty(NULL);
  if ( name == "visible" )
  {
    newProperty = new mitk::BoolProperty(true);
  }
  else
  {
    //std::cout << "Warning: non-existing default property requested: " << name << std::endl;
  }
  return newProperty;
}
bool mitk::PropertyManager::GetDefaultLimits(const std::string &name,std::pair<float,float> &minMax)
{
  PropertyLimitsMap::iterator it = m_PropertyLimits.find(name.c_str());
  if (it != m_PropertyLimits.end())
  {
    minMax = it->second;
    return true;
  }
  else
  {
    minMax = std::make_pair(std::numeric_limits<float>::min(),std::numeric_limits<float>::max());
    return false;
  }
}
