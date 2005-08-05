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


#include "mitkBaseProperty.h"
#include <mitkXMLWriter.h>

// XML
std::string mitk::BaseProperty::VALUE = "VALUE";
const std::string mitk::BaseProperty::XML_NODE_NAME = "property";

//##ModelId=3E38FF0902A7
mitk::BaseProperty::BaseProperty() 
{
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
}


//##ModelId=3E38FF0902CF
mitk::BaseProperty::~BaseProperty()
{
}

std::string mitk::BaseProperty::GetValueAsString() const
{
   return "n/a"; 
}

bool mitk::BaseProperty::WriteXMLData( XMLWriter& xmlWriter )
{
  xmlWriter.WriteProperty( VALUE, GetValueAsString() );
  return true;
}

bool mitk::BaseProperty::ReadXMLData( XMLReader& xmlReader )
{
  return false;
}

const std::string& mitk::BaseProperty::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}
