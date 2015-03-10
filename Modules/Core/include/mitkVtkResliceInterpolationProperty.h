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

#ifndef _MITK_VTK_RESLICE_INTERPOLATION_PROPERTY__H_
#define _MITK_VTK_RESLICE_INTERPOLATION_PROPERTY__H_

#include "mitkEnumerationProperty.h"

#include <vtkImageReslice.h>

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the enumeration for reslice interpolation. Valid values are
 * (VTK constant/Id/string representation):
 * VTK_RESLICE_NEAREST, VTK_RESLICE_LINEAR, VTK_RESLICE_CUBIC
 * Default is VTK_RESLICE_NEAREST
 */
class MITKCORE_EXPORT VtkResliceInterpolationProperty : public EnumerationProperty
{
public:

  mitkClassMacro( VtkResliceInterpolationProperty, EnumerationProperty );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitkNewMacro1Param(VtkResliceInterpolationProperty, const IdType&);

  mitkNewMacro1Param(VtkResliceInterpolationProperty, const std::string&);

  /**
   * Returns the current interpolation value as defined by VTK constants.
   */
  virtual int GetInterpolation();

  /**
   * Sets the interpolation type to VTK_RESLICE_NEAREST.
   */
  virtual void SetInterpolationToNearest();

  /**
   * Sets the interpolation type to VTK_RESLICE_LINEAR.
   */
  virtual void SetInterpolationToLinear();

  /**
   * Sets the interpolation type to VTK_RESLICE_CUBIC.
   */
  virtual void SetInterpolationToCubic();

  using BaseProperty::operator=;

protected:

  /** Sets reslice interpolation mode to default (VTK_RESLICE_NEAREST).
   */
  VtkResliceInterpolationProperty( );

  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  VtkResliceInterpolationProperty( const IdType& value );

  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  VtkResliceInterpolationProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid interpolation types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id );

  /**
   * Adds the enumeration types as defined by vtk to the list of known
   * enumeration values.
   */
  virtual void AddInterpolationTypes();

private:

  // purposely not implemented
  VtkResliceInterpolationProperty& operator=(const VtkResliceInterpolationProperty&);

  virtual itk::LightObject::Pointer InternalClone() const;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif


