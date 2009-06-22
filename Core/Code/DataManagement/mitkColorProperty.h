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


#ifndef MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1
#define MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include <itkRGBPixel.h>

namespace mitk {

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
  
  ColorProperty(const float red, const float green, const float blue);
    
  ColorProperty(const float color[3]);
  
  ColorProperty(const mitk::Color & color);
    
public:
  
  mitkClassMacro(ColorProperty, BaseProperty);
  
  itkNewMacro(ColorProperty);
  mitkNewMacro1Param(ColorProperty, const float*);
  mitkNewMacro1Param(ColorProperty, const mitk::Color&);
  mitkNewMacro3Param(ColorProperty, const float, const float, const float);
  
  virtual ~ColorProperty();
   
  virtual bool Assignable(const BaseProperty& other) const;
  virtual BaseProperty& operator=(const BaseProperty& other);
 
  virtual bool operator==(const BaseProperty& property) const;
  
  const mitk::Color & GetColor() const;
  const mitk::Color & GetValue() const;
  std::string GetValueAsString() const;
  void SetColor(const mitk::Color & color );
  void SetColor( float red, float green, float blue );

};

} // namespace mitk

#endif /* MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1 */
