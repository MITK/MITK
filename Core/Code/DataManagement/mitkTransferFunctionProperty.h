/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED
#define MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED

#include "mitkBaseProperty.h"
#include "mitkTransferFunction.h"


namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

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
  TransferFunctionProperty(const TransferFunctionProperty& other);

  TransferFunctionProperty( mitk::TransferFunction::Pointer value );

private:

  // purposely not implemented
  TransferFunctionProperty& operator=(const TransferFunctionProperty&);

  itk::LightObject::Pointer InternalClone() const;

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty& property);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif /* MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED */
