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


#include "mitkLookupTableProperty.h"


//##ModelId=3ED953090121
mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable &lut)
{
		std::cout << "created new mitk::LookupTableProperty..." << std::endl;
		this->SetLookupTable(lut);
}

////##ModelId=3EF198D9012D
//mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable &aLookupTable)
//{
//    SetLookupTable(aLookupTable);
//}

//##ModelId=3ED953090122
mitk::LookupTableProperty::~LookupTableProperty()
{
}

//##ModelId=3ED953090124
bool mitk::LookupTableProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_LookupTable==m_LookupTable;

}

//##ModelId=3ED953090133
mitk::LookupTable & mitk::LookupTableProperty::GetLookupTable() 
{
    return m_LookupTable;
}

//##ModelId=3ED953090135
void mitk::LookupTableProperty::SetLookupTable(const mitk::LookupTable &aLookupTable)
{
//		std::cout << "setting LUT property ... " << std::endl;
   
    if(m_LookupTable != aLookupTable)
    {
        m_LookupTable = aLookupTable;
        Modified();
    }
      		
//		std::cout << "setting LUT property OK! " << std::endl;    
}
