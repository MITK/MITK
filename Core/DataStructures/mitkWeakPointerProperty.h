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


#ifndef MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791
#define MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include "itkWeakPointer.h"

namespace mitk {

//##ModelId=3ED94AC90382
//##Documentation
//## @brief Property containing a smart-pointer
//##
//## @ingroup DataTree
class MITK_CORE_EXPORT WeakPointerProperty : public BaseProperty
{
  public:
    mitkClassMacro(WeakPointerProperty, BaseProperty);

    mitkNewMacro1Param(WeakPointerProperty, itk::Object*);
    
    //##ModelId=3ED94B4203C1
    virtual bool operator==(const BaseProperty& property) const;

    //##ModelId=3ED94B750111
    virtual ~WeakPointerProperty();

    //##ModelId=3ED952AD02B7
    itk::Object::Pointer GetWeakPointer() const;

  //##ModelId=3ED952AD02F6
    void SetWeakPointer(itk::Object* pointer);
  protected:
    //##ModelId=3ED95309021C
    itk::WeakPointer<itk::Object> m_WeakPointer;
    
    //##ModelId=3ED94B7500F2
    WeakPointerProperty(itk::Object* pointer);
};

} // namespace mitk



#endif /* MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791 */
