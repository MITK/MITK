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

#ifndef _MITK_GRID_VOLUME_MAPPER_PROPERTY__H_
#define _MITK_GRID_VOLUME_MAPPER_PROPERTY__H_

#include "mitkEnumerationProperty.h"
#include "MitkDataTypesExtExports.h"

namespace mitk
{

/**
 * Extends the enumeration prop mitkEnumerationPropery. Adds the value
 * (constant/Id/string representation):
 * PT/0/Projected Tetrahedra, ZSWEEP/1/ZSweep, RAYCAST/2/Ray Cast
 * Default is the Wireframe representation
 */
class MITKDATATYPESEXT_EXPORT GridVolumeMapperProperty : public EnumerationProperty
{
public:

  enum MapperType {
    RAYCAST = 0,
    PT = 1,
    ZSWEEP = 2
  };


  mitkClassMacro( GridVolumeMapperProperty, EnumerationProperty );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitkNewMacro1Param(GridVolumeMapperProperty, const IdType&);

  mitkNewMacro1Param(GridVolumeMapperProperty, const std::string&);

  /**
   * Sets the mapper to projected tetrahedra.
   */
  virtual void SetVolumeMapperToPT();

  /**
   * Sets the mapper to zsweep.
   */
  virtual void SetVolumeMapperToZSweep();

  /**
   * Sets the mapper to ray casting.
   */
  virtual void SetVolumeMapperToRayCast();


protected:

  /**
   * Constructor. Sets the representation to a default value of Wireframe(1)
   */
  GridVolumeMapperProperty( );

  /**
   * Constructor. Sets the representation to the given value. If it is not
   * valid, the representation is set to Wireframe(1)
   * @param value the integer representation of the representation
   */
  GridVolumeMapperProperty( const IdType& value );

  /**
   * Constructor. Sets the representation to the given value. If it is not
   * valid, the representation is set to Wireframe(1)
   * @param value the string representation of the representation
   */
  GridVolumeMapperProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid representation types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id ) override;

  /**
   * Adds the enumeration types as defined by vtk to the list of known
   * enumeration values.
   */
  virtual void AddRepresentationTypes();

private:

  virtual itk::LightObject::Pointer InternalClone() const override;
};
} // end of namespace mitk
#endif // _MITK_GRID_VOLUME_MAPPER_PROPERTY__H_
