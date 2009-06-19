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


#include "mitkLookupTableProperty.h"
#include <mitkXMLReader.h>

mitk::LookupTableProperty::LookupTableProperty()
{
}

mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable::Pointer lut)
{
    LOG_INFO << "created mitk::LookupTableProperty::New..." << std::endl;
    this->SetLookupTable(lut);
}

//mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable &aLookupTable)
//{
//    SetLookupTable(aLookupTable);
//}

mitk::LookupTableProperty::~LookupTableProperty()
{
}

bool mitk::LookupTableProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_LookupTable==m_LookupTable;

}

void mitk::LookupTableProperty::SetLookupTable(const mitk::LookupTable::Pointer aLookupTable)
{
//    LOG_INFO << "setting LUT property ... " << std::endl;
   
    if((m_LookupTable != aLookupTable) || (*m_LookupTable != *aLookupTable))
    {
        m_LookupTable = aLookupTable;
        Modified();
    }
          
//    LOG_INFO << "setting LUT property OK! " << std::endl;    
}

bool mitk::LookupTableProperty::WriteXMLData(XMLWriter &xmlWriter)
{
    m_LookupTable->WriteXML( xmlWriter );
    return true;
}

bool mitk::LookupTableProperty::ReadXMLData(XMLReader &xmlReader)
{
  int value;

  if ( xmlReader.Goto( mitk::LookupTable::XML_NODE_NAME ) ) {
    mitk::LookupTable::Pointer lookupTable = dynamic_cast<LookupTable*>(xmlReader.CreateObject().GetPointer()); 
    lookupTable->GetVtkLookupTable()->Allocate();
    lookupTable->GetVtkLookupTable()->Build();
    if (xmlReader.GetAttribute(mitk::LookupTable::NUMBER_OF_COLORS, value))
      lookupTable->GetVtkLookupTable()->SetNumberOfColors(value);
    if (xmlReader.GetAttribute(mitk::LookupTable::SCALE, value))
      lookupTable->GetVtkLookupTable()->SetScale(value);
    if (xmlReader.GetAttribute(mitk::LookupTable::RAMP, value))
      lookupTable->GetVtkLookupTable()->SetRamp(value);
    lookupTable->ReadXMLData(xmlReader);
    xmlReader.GotoParent(); // now we are back on tag <lookupTable>
    xmlReader.GotoParent(); // now we are back on tag <property>
  }
return true;
}
