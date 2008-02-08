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


#ifndef MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#define MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#include "mitkPropertyList.h"
#include <set>
#include <map>

namespace mitk {
  class MITK_CORE_EXPORT PropertyManager {

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


