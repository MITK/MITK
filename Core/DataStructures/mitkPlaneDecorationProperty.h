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

#ifndef MITK_PLANE_DECORATION_PROPERTY__H
#define MITK_PLANE_DECORATION_PROPERTY__H

#include "mitkEnumerationProperty.h"

namespace mitk
{

/**
 * Encapsulates the enumeration vtkInterpolation. Valid values are 
 * (VTK constant/Id/string representation):
 * VTK_FLAT/0/Flat, VTK_GOURAUD/1/Gouraud, VTK_PHONG/2/Phong
 * Default is the Gouraud interpolation
 */
class MITK_CORE_EXPORT PlaneDecorationProperty : public EnumerationProperty
{
public:

  mitkClassMacro( PlaneDecorationProperty, EnumerationProperty );
  
  itkNewMacro(PlaneDecorationProperty);
  
  mitkNewMacro1Param(PlaneDecorationProperty, const IdType&);
 
  mitkNewMacro1Param(PlaneDecorationProperty, const std::string&);

  enum
  {
    PLANE_DECORATION_NONE,
    PLANE_DECORATION_POSITIVE_ORIENTATION,
    PLANE_DECORATION_NEGATIVE_ORIENTATION
  };
  
  /**
   * Returns the state of plane decoration.
   */
  virtual int GetPlaneDecoration();

  /**
   * Sets the decoration type to no decoration.
   */
  virtual void SetPlaneDecorationToNone();
  
  /**
   * Sets the decoration type to arrows in positive plane direction.
   */
  virtual void SetPlaneDecorationToPositiveOrientation();
  
  /**
   * Sets the decoration type to arrows in negative plane direction.
   */
  virtual void SetPlaneDecorationToNegativeOrientation();
  
protected:
  
  /**
   * Constructor. Sets the decoration type to none.
   */
  PlaneDecorationProperty( );
  
  /**
   * Constructor. Sets the decoration type to the given value. If it is not
   * valid, the interpolation is set to none
   */
  PlaneDecorationProperty( const IdType &value );
  
  /**
   * Constructor. Sets the decoration type to the given value. If it is not
   * valid, the representation is set to none
   */
  PlaneDecorationProperty( const std::string &value );


  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid interpolation types.
   */
  virtual bool AddEnum( const std::string &name, const IdType &id );

  /**
   * Adds the enumeration types as defined by vtk to the list of known 
   * enumeration values.
   */
  virtual void AddDecorationTypes();
};

} // end of namespace mitk


#endif
