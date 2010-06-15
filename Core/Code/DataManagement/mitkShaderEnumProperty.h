/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-14 19:45:53 +0200 (Mo, 14 Apr 2008) $
Version:   $Revision: 14081 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef __MITKSHADERENUMPROPERTY_H
#define __MITKSHADERENUMPROPERTY_H

#include "mitkEnumerationProperty.h"

namespace mitk
{

/**
 * Encapsulates the shader enumeration
 */
class MITK_CORE_EXPORT ShaderEnumProperty : public EnumerationProperty
{
public:

  class Element {
    public:
    std::string name;
  };
  
  mitkClassMacro( ShaderEnumProperty, EnumerationProperty );
  
  itkNewMacro(ShaderEnumProperty);
  
  mitkNewMacro1Param(ShaderEnumProperty, const IdType&);
  
  mitkNewMacro1Param(ShaderEnumProperty, const std::string&);
  
  /**
   * Returns the current scalar mode value as defined by VTK constants.
   * @returns the current scalar mode as VTK constant.
   */
  IdType GetShaderId();
  std::string GetShaderName();
  void SetShader(const IdType& i);
  void SetShader(const std::string& i);
  
protected:
  
  std::list<Element> shaderList;

  /**
   * Constructor. Sets the representation to a default value of surface(2)
   */
  ShaderEnumProperty( );
  
  /**
   * \brief Sets the scalar mode to the given value. If it is not
   * valid, the scalar mode is set to default (0).
   * @param value the integer representation of the scalar mode
   */
  ShaderEnumProperty( const IdType& value );
  
  /**
   * \brief Sets the scalar mode to the given value. If it is not
   * valid, the representation is set to default (0).
   * @param value the string representation of the scalar mode
   */
  ShaderEnumProperty( const std::string& value );
  
   /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid scalar mode types.
   */
  bool AddEnum( const std::string& name );

  /**
   * Adds the enumeration types as defined by vtk to the list of known 
   * enumeration values.
   */
  void AddShaderTypes();
  
};

} // end of namespace mitk

#endif //_MITK_VTK_SCALARMODE_PROPERTY__H_


