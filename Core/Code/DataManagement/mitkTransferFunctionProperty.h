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


#ifndef MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED
#define MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED

#include "mitkBaseProperty.h"
#include "mitkTransferFunction.h"


namespace mitk {

class MITK_CORE_EXPORT TransferFunctionProperty : public BaseProperty
{
public:

  typedef mitk::TransferFunction::Pointer ValueType;

  mitkClassMacro(TransferFunctionProperty, BaseProperty);

  itkNewMacro(TransferFunctionProperty);
  mitkNewMacro1Param(TransferFunctionProperty, mitk::TransferFunction::Pointer);

  itkSetMacro(Value, mitk::TransferFunction::Pointer );
  itkGetConstMacro(Value, mitk::TransferFunction::Pointer );

  std::string GetValueAsString() const;

  using BaseProperty::operator=;

protected:
  mitk::TransferFunction::Pointer m_Value;

  TransferFunctionProperty();

  TransferFunctionProperty( mitk::TransferFunction::Pointer value );

private:

  // purposely not implemented
  TransferFunctionProperty(const TransferFunctionProperty&);
  TransferFunctionProperty& operator=(const TransferFunctionProperty&);

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty& property);
};

} // namespace mitk  

#endif /* MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED */ 
