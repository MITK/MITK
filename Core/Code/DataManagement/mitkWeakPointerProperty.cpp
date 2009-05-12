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

bool mitk::WeakPointerProperty::operator==(const BaseProperty& property) const
{
    const Self *other = dynamic_cast<const Self*>(&property);

    if(other==NULL) return false;

    return other->m_WeakPointer==m_WeakPointer;
}

mitk::WeakPointerProperty::WeakPointerProperty(itk::Object* pointer) : m_WeakPointer(pointer)
{
    Modified();
}

mitk::WeakPointerProperty::~WeakPointerProperty()
{
}


itk::Object::Pointer mitk::WeakPointerProperty::GetWeakPointer() const
{
    return m_WeakPointer.GetPointer();
}



void mitk::WeakPointerProperty::SetWeakPointer(itk::Object* pointer)
{
    if(m_WeakPointer.GetPointer() != pointer)
    {
        m_WeakPointer = pointer;
        Modified();
    }
}
