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

#include <MitkExports.h>
#include "mitkBaseProperty.h"
#include "itkWeakPointer.h"

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

  //##Documentation
  //## @brief Property containing a smart-pointer
  //##
  //## @ingroup DataManagement
  class MITK_CORE_EXPORT WeakPointerProperty : public BaseProperty
  {
  public:
    mitkClassMacro(WeakPointerProperty, BaseProperty);

    itkNewMacro(WeakPointerProperty);
    mitkNewMacro1Param(WeakPointerProperty, itk::Object*);

    virtual ~WeakPointerProperty();

    typedef itk::WeakPointer<itk::Object> ValueType;

    ValueType GetWeakPointer() const;
    ValueType GetValue() const;

    void SetWeakPointer(itk::Object* pointer);
    void SetValue(const ValueType& value);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

  protected:
    itk::WeakPointer<itk::Object> m_WeakPointer;

    WeakPointerProperty(itk::Object* pointer = 0);

  private:
    // purposely not implemented
    WeakPointerProperty(const WeakPointerProperty&);
    WeakPointerProperty& operator=(const WeakPointerProperty&);

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);
  };

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif /* MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791 */
