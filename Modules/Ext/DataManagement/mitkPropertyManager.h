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


#ifndef MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#define MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#include "mitkPropertyList.h"
#include "MitkExtExports.h"
#include <set>
#include <map>

namespace mitk {
  class MitkExt_EXPORT PropertyManager {

    public:
      typedef std::set<std::string> PropertyNameSet;

//      typedef PropertyNameSetterator PropertyNameIterator;
      const PropertyNameSet& GetDefaultPropertyNames();
      BaseProperty::Pointer CreateDefaultProperty(std::string name);
      static PropertyManager* GetInstance();
      bool GetDefaultLimits(const std::string &name,std::pair<float,float> &minMax);
    protected:
      PropertyManager();
      PropertyNameSet m_DefaultPropertyNameSet;
      static void InitDefaultPropertyNames();
      typedef std::map< std::string, std::pair<float,float> > PropertyLimitsMap;
      PropertyLimitsMap m_PropertyLimits;
  }      ;
}


#endif /* MITKPROPERTYMANAGER_H_HEADER_INCLUDED */


