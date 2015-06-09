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

/**
 * @brief The TransferFunctionProperty class Property class for the mitk::TransferFunction.
 * @ingroup DataManagement
 *
 * @note If you want to use this property for an mitk::Image, make sure
 * to set the mitk::RenderingModeProperty to a mode which supports transfer
 * functions (e.g. COLORTRANSFERFUNCTION_COLOR). Make sure to check the
 * documentation of the mitk::RenderingModeProperty. For a code example how
 * to use the mitk::TransferFunction check the
 * mitkImageVtkMapper2DTransferFunctionTest.cpp in Core\Code\Testing.
 */
class MITKCORE_EXPORT TransferFunctionProperty : public BaseProperty
{
public:

  typedef mitk::TransferFunction::Pointer ValueType;

  mitkClassMacro(TransferFunctionProperty, BaseProperty);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  mitkNewMacro1Param(TransferFunctionProperty, mitk::TransferFunction::Pointer);

  itkSetMacro(Value, mitk::TransferFunction::Pointer );
  itkGetConstMacro(Value, mitk::TransferFunction::Pointer );

  std::string GetValueAsString() const override;

  using BaseProperty::operator=;

protected:
  mitk::TransferFunction::Pointer m_Value;

  TransferFunctionProperty();
  TransferFunctionProperty(const TransferFunctionProperty& other);

  TransferFunctionProperty( mitk::TransferFunction::Pointer value );

private:

  // purposely not implemented
  TransferFunctionProperty& operator=(const TransferFunctionProperty&);

  itk::LightObject::Pointer InternalClone() const override;

  virtual bool IsEqual(const BaseProperty& property) const override;
  virtual bool Assign(const BaseProperty& property) override;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif /* MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED */
