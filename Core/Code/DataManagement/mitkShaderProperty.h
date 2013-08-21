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

#ifndef __MITKSHADERENUMPROPERTY_H
#define __MITKSHADERENUMPROPERTY_H

#include "mitkEnumerationProperty.h"

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the shader enumeration
 */
class MITK_CORE_EXPORT ShaderProperty : public EnumerationProperty
{
public:

  class Element {
    public:
    std::string name;
  };

  mitkClassMacro( ShaderProperty, EnumerationProperty );

  itkNewMacro(ShaderProperty);


  mitkNewMacro1Param(ShaderProperty, const IdType&);

  mitkNewMacro1Param(ShaderProperty, const std::string&);

  /**
   * Returns the current scalar mode value as defined by VTK constants.
   * @returns the current scalar mode as VTK constant.
   */
  IdType GetShaderId();
  std::string GetShaderName();
  void SetShader(const IdType& i);
  void SetShader(const std::string& i);

  using BaseProperty::operator=;

protected:

  std::list<Element> shaderList;

  /**
   * Constructor. Sets the representation to a default value of surface(2)
   */
  ShaderProperty( );

  ShaderProperty(const ShaderProperty& other);

  /**
   * \brief Sets the scalar mode to the given value. If it is not
   * valid, the scalar mode is set to default (0).
   * @param value the integer representation of the scalar mode
   */
  ShaderProperty( const IdType& value );

  /**
   * \brief Sets the scalar mode to the given value. If it is not
   * valid, the representation is set to default (0).
   * @param value the string representation of the scalar mode
   */
  ShaderProperty( const std::string& value );

   /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid scalar mode types.
   */
  bool AddEnum( const std::string& name, const IdType& id = 0);

  /**
   * Adds the enumeration types as defined by vtk to the list of known
   * enumeration values.
   */
  void AddShaderTypes();

private:

  // purposely not implemented
  ShaderProperty& operator=(const ShaderProperty&);

  virtual itk::LightObject::Pointer InternalClone() const;

  virtual bool Assign(const BaseProperty &property);

};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif //_MITK_VTK_SCALARMODE_PROPERTY__H_


