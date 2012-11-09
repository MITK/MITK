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

#ifndef _MITK_GRID_REPRESENTATION_PROPERTY__H_
#define _MITK_GRID_REPRESENTATION_PROPERTY__H_

#include "mitkEnumerationProperty.h"
#include "MitkExtExports.h"

namespace mitk
{

/**
 * Extends the enumeration prop mitkEnumerationPropery. Adds the value
 * (constant / Id / string representation):
 * <ul>
 * <li>POINTS / 0 / Points</li>
 * <li>WIREFRAME / 1 / Wireframe</li>
 * <li>SURFACE / 2 / Surface</li>
 * <li>WIREFRAME_SURFACE / 3 / WireframeSurface</li>
 * </ul>
 * Default is the Wireframe representation
 */
class MitkExt_EXPORT GridRepresentationProperty : public EnumerationProperty
{
public:

  enum RepresentationType {POINTS = 0, WIREFRAME = 1, SURFACE = 2}; //, WIREFRAME_SURFACE = 3};

  mitkClassMacro( GridRepresentationProperty, EnumerationProperty );

  itkNewMacro(GridRepresentationProperty);

  mitkNewMacro1Param(GridRepresentationProperty, const IdType& );

  mitkNewMacro1Param(GridRepresentationProperty, const std::string& );

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
   * Sets the representation type to WIREFRAME_SURFACE.
   */
  virtual void SetRepresentationToWireframeSurface();

protected:

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
