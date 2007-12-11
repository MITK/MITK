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


#include "mitkWeakPointerProperty.h"

//##ModelId=3ED94B4203C1
bool mitk::WeakPointerProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_WeakPointer==m_WeakPointer;
}

//##ModelId=3ED94B7500F2
mitk::WeakPointerProperty::WeakPointerProperty(itk::Object* pointer) : m_WeakPointer(pointer)
{
    Modified();
}

//##ModelId=3ED94B750111
mitk::WeakPointerProperty::~WeakPointerProperty()
{
}


//##ModelId=3ED952AD02B7
itk::Object::Pointer mitk::WeakPointerProperty::GetWeakPointer() const
{
    return m_WeakPointer.GetPointer();
}



//##ModelId=3ED952AD02F6
void mitk::WeakPointerProperty::SetWeakPointer(itk::Object* pointer)
{
    if(m_WeakPointer.GetPointer() != pointer)
    {
        m_WeakPointer = pointer;
        Modified();
    }
}
