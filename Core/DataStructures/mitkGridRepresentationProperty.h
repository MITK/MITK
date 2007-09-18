/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _MITK_GRID_REPRESENTATION_PROPERTY__H_
#define _MITK_GRID_REPRESENTATION_PROPERTY__H_

#include "mitkEnumerationProperty.h"

namespace mitk
{

/**
 * Extends the enumeration prop mitkEnumerationPropery. Adds the value 
 * (constant/Id/string representation):
 * POINTS/0/Points, WIREFRAME/1/Wireframe, SURFACE/2/Surface, VOLUME/3/Volume
 * Default is the Wireframe representation
 */
class GridRepresentationProperty : public EnumerationProperty
{
public:

  enum RepresentationType {POINTS = 0, WIREFRAME = 1, SURFACE = 2, VOLUME = 3};

  mitkClassMacro( GridRepresentationProperty, EnumerationProperty );
  
  /**
   * Constructor. Sets the representation to a default value of Wireframe(1)
   */
  GridRepresentationProperty( );
  
  /**
   * Constructor. Sets the representation to the given value. If it is not
   * valid, the representation is set to Wireframe(1)
   * @param value the integer representation of the representation
   */
  GridRepresentationProperty( const IdType& value );
  
  /**
   * Constructor. Sets the representation to the given value. If it is not
   * valid, the representation is set to Wireframe(1)
   * @param value the string representation of the representation
   */
  GridRepresentationProperty( const std::string& value );

  /**
   * Sets the representation type to POINTS.
   */
  virtual void SetRepresentationToPoints();
  
  /**
   * Sets the representation type to WIREFRAME.
   */
  virtual void SetRepresentationToWireframe();
  
  /**
   * Sets the representation type to SURFACE.
   */
  virtual void SetRepresentationToSurface();
  
  /**
   * Sets the representation type to VOLUME.
   */
  virtual void SetRepresentationToVolume();
  
protected:

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
#endif // _MITK_GRID_REPRESENTATION_PROPERTY__H_
