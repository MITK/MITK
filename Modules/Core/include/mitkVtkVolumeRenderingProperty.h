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

#ifndef _MITK_VTK_VOLUME_RENDERING_PROPERTY__H_
#define _MITK_VTK_VOLUME_RENDERING_PROPERTY__H_

#include "mitkEnumerationProperty.h"

#define VTK_RAY_CAST_COMPOSITE_FUNCTION 1
#define VTK_VOLUME_RAY_CAST_MIP_FUNCTION 2
namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the enumeration for volume rendering. Valid values are
 * (VTK constant/Id/string representation):
 * VTK_VOLUME_RAY_CAST_MIP_FUNCTION
 * VTK_RAY_CAST_COMPOSITE_FUNCTION
 * Default is NULL
 */
class MITKCORE_EXPORT VtkVolumeRenderingProperty : public EnumerationProperty
{
public:

  mitkClassMacro( VtkVolumeRenderingProperty, EnumerationProperty );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitkNewMacro1Param(VtkVolumeRenderingProperty, const IdType&);

  mitkNewMacro1Param(VtkVolumeRenderingProperty, const std::string&);

  /**
   * Returns the current volume rendering type
   */
  virtual int GetRenderingType();

  /**
   * Sets the rendering type to VTK_VOLUME_RAY_CAST_MIP_FUNCTION
   */
  virtual void SetRenderingTypeToMIP();

  /**
   * Sets the rendering type to VTK_RAY_CAST_COMPOSITE_FUNCTION
   */
  virtual void SetRenderingTypeToComposite();

  using BaseProperty::operator=;

 protected:

  /** Sets rendering type to default (VTK_RAY_CAST_COMPOSITE_FUNCTION).
   */
  VtkVolumeRenderingProperty( );

  /**
   * Constructor. Sets rendering type to the given value.
   */
  VtkVolumeRenderingProperty( const IdType& value );

  /**
   * Constructor. Sets rendering type to the given value.
   */
  VtkVolumeRenderingProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid rendering types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id ) override;

  /**
   * Adds the enumeration types as defined by vtk to the list of known
   * enumeration values.
   */
  virtual void AddRenderingTypes();

private:

  // purposely not implemented
  VtkVolumeRenderingProperty& operator=(const VtkVolumeRenderingProperty&);

  itk::LightObject::Pointer InternalClone() const override;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif


