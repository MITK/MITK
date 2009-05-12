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

#ifndef _MITK_VTK_REPRESENTATION_PROPERTY__H_
#define _MITK_VTK_REPRESENTATION_PROPERTY__H_

#include "mitkEnumerationProperty.h"

namespace mitk
{

/**
 * Encapsulates the enumeration vtkRepresentation. Valid values are 
 * (VTK constant/Id/string representation):
 * VTK_POINTS/0/Points, VTK_WIREFRAME/1/Wireframe, VTK_SURFACE/2/Surface
 * Default is the Surface representation
 */
class MITK_CORE_EXPORT VtkRepresentationProperty : public EnumerationProperty
{
public:

  mitkClassMacro( VtkRepresentationProperty, EnumerationProperty );
 
  itkNewMacro(VtkRepresentationProperty);
  
  mitkNewMacro1Param(VtkRepresentationProperty, const IdType&);
 
  mitkNewMacro1Param(VtkRepresentationProperty, const std::string&);
  
  /**
   * Returns the current representation value as defined by VTK constants.
   * @returns the current representation as VTK constant.
   */
  virtual int GetVtkRepresentation();

  /**
   * Sets the representation type to VTK_POINTS.
   */
  virtual void SetRepresentationToPoints();
  
  /**
   * Sets the representation type to VTK_WIREFRAME.
   */
  virtual void SetRepresentationToWireframe();
  
  /**
   * Sets the representation type to VTK_SURFACE.
   */
  virtual void SetRepresentationToSurface();
  
protected:
  
  /**
   * Constructor. Sets the representation to a default value of Surface(2)
   */
  VtkRepresentationProperty( );
  
  /**
   * Constructor. Sets the representation to the given value. If it is not
   * valid, the representation is set to Surface(2)
   * @param value the integer representation of the representation
   */
  VtkRepresentationProperty( const IdType& value );
  
  /**
   * Constructor. Sets the representation to the given value. If it is not
   * valid, the representation is set to Surface(2)
   * @param value the string representation of the representation
   */
  VtkRepresentationProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid representation types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id );

  /**
   * Adds the enumeration types as defined by vtk to the list of known 
   * enumeration values.
   */
  virtual void AddRepresentationTypes();
};
} // end of namespace mitk
#endif


