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


#ifndef MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#define MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#include "mitkPropertyList.h"
#include <set>
namespace mitk {
  class PropertyManager {

    public:
      typedef std::set<std::string> PropertyNameSet; 
//      typedef PropertyNameSetterator PropertyNameIterator;
      static const PropertyNameSet& GetDefaultPropertyNames();
      static BaseProperty::Pointer CreateDefaultProperty(std::string name);

    protected:
      PropertyManager();
      PropertyNameSet m_DefaultPropertyNameSet;
      static void InitDefaultPropertyNames();
  }      ;
}  


#endif /* MITKPROPERTYMANAGER_H_HEADER_INCLUDED */

