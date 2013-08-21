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


#ifndef MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1
#define MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1

#include <MitkExports.h>
#include "mitkBaseProperty.h"
#include <itkRGBPixel.h>

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

//##Documentation
//## @brief Standard RGB color typedef (float)
//##
//## Standard RGB color typedef to get rid of template argument (float).
//## @ingroup Property
typedef itk::RGBPixel< float > Color;

//##Documentation
//## @brief RGB color property
//##
//## @ingroup DataManagement
class MITK_CORE_EXPORT ColorProperty : public BaseProperty
{
protected:
  mitk::Color m_Color;

  ColorProperty();

  ColorProperty(const ColorProperty& other);

  ColorProperty(const float red, const float green, const float blue);

  ColorProperty(const float color[3]);

  ColorProperty(const mitk::Color & color);

public:

  mitkClassMacro(ColorProperty, BaseProperty)

  itkNewMacro(ColorProperty);
  mitkNewMacro1Param(ColorProperty, const float*);
  mitkNewMacro1Param(ColorProperty, const mitk::Color&);
  mitkNewMacro3Param(ColorProperty, const float, const float, const float);

  typedef mitk::Color ValueType;

  const mitk::Color & GetColor() const;
  const mitk::Color & GetValue() const;
  std::string GetValueAsString() const;
  void SetColor(const mitk::Color & color );
  void SetValue(const mitk::Color & color );
  void SetColor( float red, float green, float blue );

  using BaseProperty::operator=;

private:

  // purposely not implemented
  ColorProperty& operator=(const ColorProperty&);

  virtual itk::LightObject::Pointer InternalClone() const;

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty & property);

};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif /* MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1 */
