/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _MITK_VTK_RESLICE_INTERPOLATION_PROPERTY__H_
#define _MITK_VTK_RESLICE_INTERPOLATION_PROPERTY__H_

#include "mitkEnumerationProperty.h"

#include <vtkImageReslice.h>

namespace mitk
{

/**
 * Encapsulates the enumeration for reslice interpolation. Valid values are 
 * (VTK constant/Id/string representation):
 * VTK_RESLICE_NEAREST, VTK_RESLICE_LINEAR, VTK_RESLICE_CUBIC
 * Default is VTK_RESLICE_NEAREST
 */
class MITK_CORE_EXPORT VtkResliceInterpolationProperty : public EnumerationProperty
{
public:

  mitkClassMacro( VtkResliceInterpolationProperty, EnumerationProperty );

  itkNewMacro(VtkResliceInterpolationProperty);
  
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
  VtkResliceInterpolationProperty(const VtkResliceInterpolationProperty&);
  VtkResliceInterpolationProperty& operator=(const VtkResliceInterpolationProperty&);
};

} // end of namespace mitk

#endif


