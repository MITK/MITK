/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSmartPointerProperty.h"

//##ModelId=3ED94B4203C1
bool mitk::SmartPointerProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_SmartPointer==m_SmartPointer;
}

//##ModelId=3ED94B7500F2
mitk::SmartPointerProperty::SmartPointerProperty(itk::Object* pointer) : m_SmartPointer(pointer)
{
    Modified();
}

//##ModelId=3ED94B750111
mitk::SmartPointerProperty::~SmartPointerProperty()
{
}

//##ModelId=3ED952AD02B7
itk::Object::Pointer mitk::SmartPointerProperty::GetSmartPointer() const
{
    return m_SmartPointer;
}

//##ModelId=3ED952AD02F6
void mitk::SmartPointerProperty::SetSmartPointer(itk::Object* pointer)
{
    if(m_SmartPointer.GetPointer() != pointer)
    {
        m_SmartPointer = pointer;
        Modified();
    }
}
