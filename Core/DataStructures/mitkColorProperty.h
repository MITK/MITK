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

//##ModelId=3E86AABB0082
//##Documentation
//## @brief Standard RGB color typedef (float)
//##
//## Standard RGB color typedef to get rid of template argument (float).
//## @ingroup Property
typedef itk::RGBPixel< float > Color;

//##ModelId=3E86A20C00FD
//##Documentation
//## @brief RGB color property
//##
//## @ingroup DataTree
class ColorProperty : public BaseProperty
{
protected:
  //##ModelId=3E86A3B00061
  mitk::Color m_Color;
public:
  ColorProperty();
  mitkClassMacro(ColorProperty, BaseProperty);
  
  ColorProperty(const float red, const float green, const float blue);
  
  //##ModelId=3E86A3450130
  ColorProperty(const float color[3]);
  
  //##ModelId=3ED9530801BD
  ColorProperty(const mitk::Color & color);
  
  //##ModelId=3E86A345014E
  virtual ~ColorProperty();
   
  virtual bool Assignable(const BaseProperty& other) const;
  virtual BaseProperty& operator=(const BaseProperty& other);
 
  //##ModelId=3E86A35F000B
  virtual bool operator==(const BaseProperty& property) const;
  
  //##ModelId=3E86AABB0371
  const mitk::Color & GetColor() const;
  //##ModelId=3E86AABB0399
  const mitk::Color & GetValue() const;
  std::string GetValueAsString() const;
  void SetColor(const mitk::Color & color );
  void SetColor( float red, float green, float blue );

  //## 
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );
};

} // namespace mitk

#endif /* MITKCOLORPROPERTY_H_HEADER_INCLUDED_C17953D1 */
